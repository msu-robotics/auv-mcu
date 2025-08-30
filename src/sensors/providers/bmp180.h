#pragma once

#include "sensors/depth/modular.h"
#include "sensors/pressure/modular.h"
#include <Wire.h>
#include "utils.h"
#include <Adafruit_BMP085.h>


class BMP180Provider : public IDepthProvider, public ITemperatureProvider, public IPressureProvider {
    Adafruit_BMP085 sensor;
    int sda, scl;
    bool initialized = false;

public:
    BMP180Provider(int sda_pin = 21, int scl_pin = 22)
        : sda(sda_pin), scl(scl_pin) {}

    bool begin() override {
        if (initialized) return true;
        char buffer[50];
        sprintf(buffer, "ℹ️ BMP initialize sda: %d scl: %d", sda, scl);
        uartDebug(buffer);


        if (!sensor.begin()) {
            uartDebug("❌ BMP180 init failed");
            initialized = false;
        }
        uartDebug("✅ BMP180 initialized");
        initialized = true;
        return true;
    }

    float readTemperature() override {
        return sensor.readTemperature();
    }

    float readDepth() override {
        return sensor.readAltitude();
    }

    float readPressure() override {
        return sensor.readPressure();
    }

};
