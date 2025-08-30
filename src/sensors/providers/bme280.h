#pragma once

#include "sensors/depth/modular.h"
#include "sensors/pressure/modular.h"
#include <Wire.h>
#include "utils.h"
#include <GyverBME280.h>


class BME280Provider : public ITemperatureProvider, public IPressureProvider {
    GyverBME280 sensor;
    int sda, scl;
    uint8_t address;
    bool initialized = false;

public:
    BME280Provider(int sda_pin = 21, int scl_pin = 22, uint8_t address = 0x77)
        : sda(sda_pin), scl(scl_pin), address(address) {}

    bool begin() override {
        if (initialized) return true;
        char buffer[50];
        sprintf(buffer, "ℹ️ BME280 initialize sda: %d scl: %d", sda, scl);
        uartDebug(buffer);

        // sensor.setHumOversampling(MODULE_DISABLE);
        // sensor.setMode(FORCED_MODE);
        sensor.setStandbyTime(STANDBY_500MS);

        if (!sensor.begin(address)) {
            uartDebug("❌ BME280 init failed");
            initialized = false;
        }
        uartDebug("✅ BME280 initialized");
        initialized = true;
        return true;
    }

    float readTemperature() override {
        return sensor.readTemperature();
    }

    float readPressure() override {
        return sensor.readPressure();
    }

};
