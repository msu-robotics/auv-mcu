#include "factory.h"
#include "utils.h"

ModularIMU* createIMU() {
    auto imu = new ModularIMU(nullptr, nullptr, nullptr, nullptr, nullptr);

    #if defined(IMU_MPU6050)
        Serial.println("ℹ️ Using MPU6050 IMU driver");
        auto mpu = new MPU6050Provider();
        imu->setAccelProvider(static_cast<IAccelProvider*>(mpu));
        imu->setAngularVelProvider(static_cast<IAngleVelocityProvider*>(mpu));
        imu->setQuaternionProvider(static_cast<IQuaternionProvider*>(mpu));
    #elif defined(IMU_HWT905)
        Serial.println("ℹ️ Using HWT905 IMU driver");
        auto mpu = new HWT905Provider(26, 27);
        imu->setAccelProvider(static_cast<IAccelProvider*>(mpu));
        imu->setAngularVelProvider(static_cast<IAngleVelocityProvider*>(mpu));
        imu->setQuaternionProvider(static_cast<IQuaternionProvider*>(mpu));
    #else
        Serial.println("❌IMU not specified");
    #endif

    #if defined(MAG_QMC5883L)
        Serial.println("ℹ️ Using QMC5883L MAG driver");
        auto qmc = new QMC5883LProvider();
        imu->setMagneticProvider(static_cast<IMagneticProvider*>(qmc));
    #elif defined(MAG_HWT905)
        Serial.println("ℹ️ Using HWT905 MAG driver");

    #else
        Serial.println("❌ MAGnetometer not specified");
    #endif

        if (!imu->begin()) {
            Serial.println("❌ IMU init failed!");
            while (true);
        }

    return imu;
}
