#pragma once

// Ускорение по осям м/с² (акселерометр)
struct AccelData { float x, y, z; };
// Значение магнитного поля по осям в µT (магнитометр)
struct MagneticData { float x, y, z; };
// Угол поворота датчика вокруг осей от -180 до 180
struct AngleData { float x, y, z; };
// Угловая скорость по осям рад/с (гироскоп)
struct AngularVelData { float x, y, z; };
// Квантернион (положение датчика в пространстве)
struct QuaternionData { float w, x, y, z; };

class IAccelProvider {
public:
    virtual bool begin() = 0;
    virtual AccelData readAccel() = 0;
    virtual ~IAccelProvider() = default;
};

class IMagneticProvider {
public:
    virtual bool begin() = 0;
    virtual MagneticData readMag() = 0;
    virtual ~IMagneticProvider() = default;
};

class IAngleProvider {
public:
    virtual bool begin() = 0;
    virtual AngleData readAngle() = 0;
    virtual ~IAngleProvider() = default;
};

class IAngleVelocityProvider {
public:
    virtual bool begin() = 0;
    virtual AngularVelData readAngularVel() = 0;
    virtual ~IAngleVelocityProvider() = default;
};

class IQuaternionProvider {
public:
    virtual bool begin() = 0;
    virtual QuaternionData readQuaternion() = 0;
    virtual ~IQuaternionProvider() = default;
};

