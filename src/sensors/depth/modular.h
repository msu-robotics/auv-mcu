#pragma once
#include "interface.h"
#include "parts.h"

class ModularDepthSensor : public DepthSensor {
    IDepthProvider* depth = nullptr;
    ITemperatureProvider* temp = nullptr;

public:
    bool ready;
    ModularDepthSensor(IDepthProvider* d, ITemperatureProvider* t)
        : depth(d), temp(t) {}

    bool begin() override {
        bool ok = true;
        if (depth) ok &= depth->begin();
        if (temp) ok &= temp->begin();
        ready = ok;
        return ok;
    }

    DepthData read() override {
        DepthData d;
        if (depth) {
            auto a = depth->readDepth();
            d.depth = a;
        }
        if (temp) {
            auto t = temp->readTemperature();
            d.temperature = t;
        }
        return d;
    }

    void setTemperatureProvider(ITemperatureProvider* newTemp, bool destroyOld = true) {
        if (destroyOld && temp) delete temp;
        temp = newTemp;
        if (temp) depth->begin();
    }

    void setDepthProvider(IDepthProvider* newDepth, bool destroyOld = true) {
        if (destroyOld && depth) delete depth;
        depth = newDepth;
        if (depth) depth->begin();
    }

    ~ModularDepthSensor() override {
        delete temp;
        delete depth;
    }
};
