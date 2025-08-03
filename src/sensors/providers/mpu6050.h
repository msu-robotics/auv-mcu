#pragma once

#include "sensors/imu/parts.h"
#include <Wire.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include "utils.h"

#define EARTH_GRAVITY_MS2 9.80665  //m/s2
#define DEG_TO_RAD        0.017453292519943295769236907684886
#define RAD_TO_DEG        57.295779513082320876798154814105


class MPU6050Provider : public IAccelProvider, public IAngleVelocityProvider, public IQuaternionProvider {
    MPU6050 mpu;
    int sda, scl;
    uint8_t devStatus;
    bool DMPReady = false;
    uint8_t MPUIntStatus;
    uint16_t packetSize;
    uint8_t FIFOBuffer[64];
    Quaternion q;
    VectorFloat gravity;
    float ypr[3];
    bool initialized = false;

public:
    MPU6050Provider(int sda_pin = 21, int scl_pin = 22)
        : sda(sda_pin), scl(scl_pin) {}

    bool begin() override {
        if (initialized)
            return true;

        char buffer[50];
        sprintf(buffer, "ℹ️ MPU6050 initialize sda: %d scl: %d", sda, scl);
        uartDebug(buffer);

        Wire.begin(sda, scl);
        delay(100);
        mpu.initialize();

        if (!mpu.testConnection()) {
            uartDebug("❌MPU6050 Initialization failed");

            return false;
        }
        devStatus = mpu.dmpInitialize();

        if (devStatus == 0) {
            mpu.CalibrateAccel(6);
            mpu.CalibrateGyro(6);

            // auto offsets = mpu.GetActiveOffsets();
            // char buffer[50];
            // sprintf(buffer, "ℹ️MPU6050 These are the Active offsets: %f, %f, %f, %f, %f, %f", offsets[0], offsets[1], offsets[2], offsets[3], offsets[4], offsets[5]);
            // uartDebug(buffer);

            uartDebug("ℹ️ MPU6050 Enabling DMP...");
            mpu.setDMPEnabled(true);

            MPUIntStatus = mpu.getIntStatus();

            uartDebug("ℹ️ MPU6050 DMP ready! Waiting for first interrupt...");
            DMPReady = true;
            packetSize = mpu.dmpGetFIFOPacketSize();
        }

        uartDebug("✅MPU6050 Initialized");
        initialized = true;
        return true;
    }

    AccelData readAccel() override {
        VectorInt16 aa;
        mpu.dmpGetCurrentFIFOPacket(FIFOBuffer);
        mpu.dmpGetAccel(&aa, FIFOBuffer);
        return { float(aa.x), float(aa.y), float(aa.z) };
    }

    AngularVelData readAngularVel() override {
        VectorFloat gg;
        mpu.dmpGetCurrentFIFOPacket(FIFOBuffer);
        mpu.dmpGetQuaternion(&q, FIFOBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        // mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        return { gravity.x, gravity.y, gravity.z };
    }

    QuaternionData readQuaternion() override {
        mpu.dmpGetCurrentFIFOPacket(FIFOBuffer);
        mpu.dmpGetQuaternion(&q, FIFOBuffer);
        return { q.w, q.x, q.y, q.z };
    }
};