#pragma once


class IDepthProvider {
public:
    virtual bool begin() = 0;
    virtual float readDepth() = 0;
    virtual ~IDepthProvider() = default;
};

class ITemperatureProvider {
public:
    virtual bool begin() = 0;
    virtual float readTemperature() = 0;
    virtual ~ITemperatureProvider() = default;
};