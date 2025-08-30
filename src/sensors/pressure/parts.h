#pragma once


class IPressureProvider {
public:
    virtual bool begin() = 0;
    virtual float readPressure() = 0;
    virtual ~IPressureProvider() = default;
};