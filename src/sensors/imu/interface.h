#pragma once

struct IMUData {
    float accelX, accelY, accelZ;
    float velX, velY, velZ;
    float magX, magY, magZ;
    float angleX, angleY, angleZ;
    float qW, qX, qY, qZ;
};

class IMU {
public:
    virtual bool begin() = 0;
    virtual IMUData read() = 0;
    virtual ~IMU() = default;
};
