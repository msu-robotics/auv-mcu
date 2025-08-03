#pragma once

struct DepthData {
    float depth;
    float temperature;
};

class DepthSensor {
public:
    virtual bool begin() = 0;
    virtual DepthData read() = 0;
    virtual ~DepthSensor() = default;
};
