#pragma once

#include "sensors/depth/modular.h"
#include <Wire.h>
#include "utils.h"
#include <MS5837.h>


#define TEMP_IS_ACTUAL   (1 << 0)
#define DEPTH_IS_ACTUAL  (1 << 1)


class MS5837Provider : public IDepthProvider, public ITemperatureProvider {
    MS5837 sensor;
    int sda, scl;
    bool initialized = false;
    uint8_t data_is_actual = 0;

public:
    MS5837Provider(int sda_pin = 33, int scl_pin = 22)
        : sda(sda_pin), scl(scl_pin) {}

    bool begin() override {
        char buffer[50];
        sprintf(buffer, "ℹ️ MS5837 initialize sda: %d scl: %d", sda, scl);
        uartDebug(buffer);

        Wire.begin(sda, scl);

        const int max_attempts = 3;
        int attempts = 0;

        while (attempts < max_attempts && !sensor.init()) {
            uartDebug("❌ MS5837 init failed");
            attempts++;
            delay(2000);
        }

        if (attempts < max_attempts) {
            uartDebug("✅ MS5837 initialized");
            sensor.setModel(MS5837::MS5837_30BA);
            sensor.setFluidDensity(997); // freshwater; use 1029 for seawater
            initialized = true;
        } else {
            uartDebug("⚠️ MS5837 unavailable — running without it");
            initialized = false;
        }
        return true;
    }

    float readTemperature() override {
        if (!(data_is_actual & TEMP_IS_ACTUAL)) {
            updateData();
        }
        data_is_actual &= ~TEMP_IS_ACTUAL;
        return sensor.temperature();
    }

    float readDepth() override {
        if (!(data_is_actual & DEPTH_IS_ACTUAL)){
            updateData();
        }
        data_is_actual &= ~ DEPTH_IS_ACTUAL;
        return sensor.depth();
    }
private:
        void updateData(void){
            data_is_actual = TEMP_IS_ACTUAL | DEPTH_IS_ACTUAL;
            sensor.read();
        }

};
