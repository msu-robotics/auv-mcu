#pragma once

class ITemperatureProvider {
public:
    virtual bool begin() = 0;
    virtual float readTemperature() = 0;
    virtual ~ITemperatureProvider() = default;
};