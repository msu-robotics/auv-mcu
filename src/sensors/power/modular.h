#pragma once
#include "interface.h"

#include "parts.h"

class ModularPowerSensor : public PowerSensor {
    ICurrentProvider* current = nullptr;
    IVoltageProvider* voltage = nullptr;

public:
    bool ready;
    ModularPowerSensor(ICurrentProvider* c, IVoltageProvider* v)
        : current(c), voltage(v) {}

    bool begin() override {
        bool ok = true;
        if (voltage) ok &= voltage->begin();
        if (current) ok &= current->begin();
        ready = ok;
        return ok;
    }

    PowerData read() override {
        PowerData p;
        if (voltage) {
            auto v = voltage->readVoltage();
            p.voltage = v;
        }
        if (current) {
            auto c = current->readCurrent();
            p.current = c;
        }
        return p;
    }

    void setVoltageProvider(IVoltageProvider* newVoltage, bool destroyOld = true) {
        if (destroyOld && voltage) delete voltage;
        voltage = newVoltage;
        if (voltage) voltage->begin();
    }

    void setCurrentProvider(ICurrentProvider* newCurrent, bool destroyOld = true) {
        if (destroyOld && current) delete current;
        current = newCurrent;
        if (current) current->begin();
    }

    ~ModularPowerSensor() override {
        delete current;
        delete voltage;
    }
};
