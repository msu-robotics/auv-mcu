#pragma once

struct PressureData {
    float pressure;
    float temperature;
};

class PressureSensor {
public:
    virtual bool begin() = 0;
    virtual PressureData read() = 0;
    virtual ~PressureSensor() = default;
};
