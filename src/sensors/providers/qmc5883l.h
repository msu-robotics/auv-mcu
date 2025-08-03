#pragma once

#include "sensors/imu/modular.h"
#include <QMC5883LCompass.h>
#include <Wire.h>
#include "utils.h"

class QMC5883LProvider : public IMagneticProvider {
    QMC5883LCompass compass;
    int sda, scl;

public:
    QMC5883LProvider(int sda_pin = 21, int scl_pin = 22)
        : sda(sda_pin), scl(scl_pin) {}

    bool begin() override {
        char buffer[50];
        sprintf(buffer, "ℹ️ QMC5883L initialize sda: %d scl: %d", sda, scl);
        uartDebug(buffer);

        Wire.begin(sda, scl);
        delay(100);
        compass.init();
        compass.read();
        if (compass.getX() == 0) {
            uartDebug("❌QMC5883L Initialization failed");

            return false;
        }
        uartDebug("✅QMC5883L Initialized");
        return true;
    }

    MagneticData readMag() override {
        MagneticData m{};
        compass.read();
        m.x = compass.getX();
        m.y = compass.getY();
        m.z = compass.getZ();
        return m;
    }
};
