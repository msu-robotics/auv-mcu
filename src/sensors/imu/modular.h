#pragma once
#include "interface.h"
#include "parts.h"

class ModularIMU : public IMU {
    IAccelProvider* accel = nullptr;
    IAngleVelocityProvider* vel = nullptr;
    IMagneticProvider* mag = nullptr;
    IAngleProvider* angle = nullptr;
    IQuaternionProvider* quaternion = nullptr;

public:
    bool ready;
    ModularIMU(IAccelProvider* a, IAngleVelocityProvider* g, IMagneticProvider* m, IAngleProvider* an, IQuaternionProvider* q)
        : accel(a), vel(g), mag(m), angle(an), quaternion(q) {}

    bool begin() override {
        bool ok = true;
        if (accel) ok &= accel->begin();
        if (vel) ok &= vel->begin();
        if (mag) ok &= mag->begin();
        if (angle) ok &= angle->begin();
        if (quaternion) ok &= quaternion->begin();
        ready = ok;
        return ok;
    }

    IMUData read() override {
        IMUData d;
        if (accel) {
            auto a = accel->readAccel();
            d.accelX = a.x; d.accelY = a.y; d.accelZ = a.z;
        }
        if (vel) {
            auto g = vel->readAngularVel();
            d.velX = g.x; d.velY = g.y; d.velZ = g.z;
        }
        if (mag) {
            auto m = mag->readMag();
            d.magX = m.x; d.magY = m.y; d.magZ = m.z;
        }
        if (angle) {
            auto a = angle->readAngle();
            d.angleX = a.x; d.angleY = a.y; d.angleZ = a.z;
        }
        if (quaternion) {
            auto q = quaternion->readQuaternion();
            d.qW = q.w; d.qX = q.x; d.qY = q.y; d.qZ = q.z;
        }
        return d;
    }

    void setAccelProvider(IAccelProvider* newAccel, bool destroyOld = true) {
        if (destroyOld && accel) delete accel;
        accel = newAccel;
        if (accel) accel->begin();
    }

    void setAngularVelProvider(IAngleVelocityProvider* newVel, bool destroyOld = true) {
        if (destroyOld && vel) delete vel;
        vel = newVel;
        if (vel) vel->begin();
    }

    void setMagneticProvider(IMagneticProvider* newMag, bool destroyOld = true) {
        if (destroyOld && mag) delete mag;
        mag = newMag;
        if (mag) mag->begin();
    }

    void setAngleProvider(IAngleProvider* newAngle, bool destroyOld = true) {
        if (destroyOld && angle) delete angle;
        angle = newAngle;
        if (angle) angle->begin();
    }

    void setQuaternionProvider(IQuaternionProvider* newQuaternion, bool destroyOld = true) {
        if (destroyOld && quaternion) delete quaternion;
        quaternion = newQuaternion;
        if (quaternion) quaternion->begin();
    }


    ~ModularIMU() override {
        delete accel;
        delete vel;
        delete mag;
        delete angle;
        delete quaternion;
    }
};
