#pragma once


class IVoltageProvider {
public:
    virtual bool begin() = 0;
    virtual float readVoltage() = 0;
    virtual ~IVoltageProvider() = default;
};


class ICurrentProvider {
public:
    virtual bool begin() = 0;
    virtual float readCurrent() = 0;
    virtual ~ICurrentProvider() = default;
};
