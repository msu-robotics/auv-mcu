#pragma once

#include "sensors/imu/parts.h"
#include <Arduino.h>
#include <utils.h>
#include <REG.h>
#include <wit_c_sdk.h>


#define ACC_IS_ACTUAL 0x01
#define ANGULAR_VEL_IS_ACTUAL 0x02
#define ANGLE_IS_ACTUAL 0x04
#define MAG_IS_ACTUAL 0x08


class HWT905Provider : public IAccelProvider, public IMagneticProvider, public IAngleVelocityProvider, public IQuaternionProvider {
    int txPin, rxPin;
    long foundBaud = 0;
    volatile uint8_t updated_data = 0;
    bool initialized = false;

    static HWT905Provider* _instance;

public:
    HWT905Provider(int tx, int rx) : txPin(tx), rxPin(rx) {
        _instance = this;
    }

    bool begin() override {
        if (initialized)
            return true;

        WitInit(WIT_PROTOCOL_NORMAL, 0x50);
        WitSerialWriteRegister(_serialSendStatic);
        WitRegisterCallBack(_dataCallbackStatic);
        WitDelayMsRegister(_delayStatic);

        if (! _autoDetectBaud()) {
            uartDebug("❌HWT905 Initialization failed");
            return false;
        }
        uartDebug("✅HWT905 Initialized");
        return true;
    }

    AccelData readAccel() override {
        if(not (updated_data & ACC_IS_ACTUAL)){
            _updateSensor();
        }
        updated_data &= ~ACC_IS_ACTUAL;
        return { fAcc[0], fAcc[1], fAcc[2] };
    }

    AngularVelData readAngularVel() override {
        if(not (updated_data & ANGULAR_VEL_IS_ACTUAL)){
            _updateSensor();
        }
        updated_data &= ~ANGLE_IS_ACTUAL;
        return { fGyro[0], fGyro[1], fGyro[2] };
    }

    MagneticData readMag() override {
        if (not updated_data & MAG_IS_ACTUAL){
            _updateSensor();
        }
        updated_data &= ~MAG_IS_ACTUAL;
        return { float(sReg[HX]), float(sReg[HY]), float(sReg[HZ]) };
    }

    QuaternionData readQuaternion() override {
        return { float(sReg[q0]), float(sReg[q1]), float(sReg[q2]), float(sReg[q3]) };
    }

private:
    float fAcc[3], fGyro[3], fAngle[3];

    void _updateSensor() {
        while (Serial1.available()) {
            WitSerialDataIn(Serial1.read());
        }
    }

    bool _autoDetectBaud() {
        const long baudList[] = {4800, 9600, 19200, 38400, 57600, 115200, 230400};

        for (long baud : baudList) {
            Serial1.begin(baud, SERIAL_8N1, rxPin, txPin);
            Serial1.flush();
            updated_data = 0;
            int retries = 2;

            while (retries--) {
                WitReadReg(AX, 3);
                delay(200);
                while (Serial1.available()) {
                    WitSerialDataIn(Serial1.read());
                }
                if (updated_data != 0) {
                    foundBaud = baud;
                    initialized = true;
                    char buffer[50];
                    sprintf(buffer, "✅HWT905 baud rate: %d", baud);
                    uartDebug(buffer);
                    return true;
                }
            }
        }
        return false;
    }

    void _serialSend(uint8_t* data, uint32_t len) {
        Serial1.write(data, len);
        Serial1.flush();
    }

    void _dataCallback(uint32_t reg, uint32_t count) {
        for (uint32_t i = 0; i < count; i++, reg++) {
            switch (reg) {
                case AZ:
                    updated_data |= ACC_IS_ACTUAL;
                    for(int i = 0; i < 3; i++) {
                        fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
                    }
                    break;
                case GZ:
                    updated_data |= ANGULAR_VEL_IS_ACTUAL;
                    for(int i = 0; i < 3; i++) {
				        fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
			        }
                    break;
                case HZ:
                    updated_data |= MAG_IS_ACTUAL;
                    break;
                case Yaw:
                    updated_data |= ANGLE_IS_ACTUAL;
                    for(int i = 0; i < 3; i++) {
				        fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
			        }
                    break;
            }
        }
    }

    void _delay(uint16_t ms) {
        delay(ms);
    }

    static void _serialSendStatic(uint8_t* data, uint32_t len) {
        if (_instance) _instance->_serialSend(data, len);
    }

    static void _dataCallbackStatic(uint32_t reg, uint32_t count) {
        if (_instance) _instance->_dataCallback(reg, count);
    }

    static void _delayStatic(uint16_t ms) {
        if (_instance) _instance->_delay(ms);
    }
};
