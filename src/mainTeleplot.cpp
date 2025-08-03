#include <Arduino.h>
#include "sensors/imu/modular.h"
#include "sensors/depth/modular.h"
#include "utils.h"
#include "sensors/imuFactory.h"
#include "sensors/depthFactory.h"


ModularIMU* imu;
ModularDepthSensor* depth;

void setup() {
    Serial.begin(115200);
    imu = createIMU();
    depth = createDepthSensor();
}

void loop() {
    // IMU Driver name
    #if defined(IMU_MPU6050)
        #define IMU
        Serial.println(">imuName:MPU6550|t");
    #elif defined(IMU_HWT905)
        #define IMU
        Serial.println(">imuName:HWT905|t");
    #endif
    // MAG Driver name
    #if defined(MAG_QMC5883L)
        #define MAG
        Serial.println(">magName:QMC5883L|t");
    #elif defined(MAG_HWT905)
        #define MAG
        Serial.println(">magName:HWT905|t");
    #endif
    // Depth Driver name
    #if defined(DEPTH_BMP180)
        #define DEPTH
        Serial.println(">depthName:BMP180|t");
    #elif defined(DEPTH_MS5837)
        #define DEPTH
        Serial.println(">depthName:MS5837|t");
    #endif

    #if defined(GIT_COMMIT_HASH)
        Serial.print(">gitHash:");
        Serial.print(GIT_COMMIT_HASH);
        Serial.println("|t");
    #endif


    IMUData i = imu->read();
    DepthData d = depth->read();

    #if defined(IMU)
        // IMU Acceleration data
        Serial.print(">accX:");
        Serial.println(i.accelX);
        Serial.print(">accY:");
        Serial.println(i.accelY);
        Serial.print(">accZ:");
        Serial.println(i.accelZ);

        // IMU Gyroscope data
        Serial.print(">velX:");
        Serial.println(i.velX);
        Serial.print(">velY:");
        Serial.println(i.velY);
        Serial.print(">velZ:");
        Serial.println(i.velZ);

        // Quaternion data
        Serial.print(">3D|angle3D:S:cube:P:1:1:1:Q:");
        Serial.print(i.qX);Serial.print(":");
        Serial.print(i.qY);Serial.print(":");
        Serial.print(i.qZ);Serial.print(":");
        Serial.print(i.qW);Serial.print(":");
        Serial.println("W:2:H:2:D:2:C:#2ecc71");
    #endif
    #if defined(MAG)
        // IMU Magnetometer data
        Serial.print(">magX:");
        Serial.println(i.magX);
        Serial.print(">magY:");
        Serial.println(i.magY);
        Serial.print(">magZ:");
        Serial.println(i.magZ);
    #endif
    #if defined(DEPTH)
        Serial.print(">depth:");
        Serial.println(d.depth);
        Serial.print(">temperature:");
        Serial.println(d.temperature);
    #endif

    delay(100);
}