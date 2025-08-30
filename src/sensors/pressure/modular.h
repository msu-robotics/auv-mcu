#pragma once
#include "interface.h"
#include "parts.h"
#include "sensors/common/temperatureProvider.h"

class ModularPressureSensor : public PressureSensor {
    IPressureProvider* pressure = nullptr;
    ITemperatureProvider* temp = nullptr;

public:
    bool ready;
    ModularPressureSensor(IPressureProvider* p, ITemperatureProvider* t)
        : pressure(p), temp(t) {}

    bool begin() override {
        bool ok = true;
        if (pressure) ok &= pressure->begin();
        if (temp) ok &= temp->begin();
        ready = ok;
        return ok;
    }

    PressureData read() override {
        PressureData p;
        if (pressure) {
            auto a = pressure->readPressure();
            p.pressure = a;
        }
        if (temp) {
            auto t = temp->readTemperature();
            p.temperature = t;
        }
        return p;
    }

    void setTemperatureProvider(ITemperatureProvider* newTemp, bool destroyOld = true) {
        if (destroyOld && temp) delete temp;
        temp = newTemp;
        if (temp) pressure->begin();
    }

    void setPressureProvider(IPressureProvider* newPressure, bool destroyOld = true) {
        if (destroyOld && pressure) delete pressure;
        pressure = newPressure;
        if (pressure) pressure->begin();
    }

    ~ModularPressureSensor() override {
        delete temp;
        delete pressure;
    }
};
