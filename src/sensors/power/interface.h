#pragma once

struct PowerData {
    float voltage;
    float current;
};

class PowerSensor {
public:
    virtual bool begin() = 0;
    virtual PowerData read() = 0;
    virtual ~PowerSensor() = default;
};
