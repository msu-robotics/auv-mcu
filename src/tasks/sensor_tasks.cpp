#include <Arduino.h>
#include <utils.h>
#include "sensor_tasks.h"
#include <Wire.h>
#include "MS5837.h"


#define ACC_UPDATE 0x01
#define GYRO_UPDATE 0x02
#define ANGLE_UPDATE 0x04
#define MAG_UPDATE 0x08
#define READ_UPDATE 0x80

static void sensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void sensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void delayMs(uint16_t ucMs);

bool imu_initialized = false;
bool depth_sensor_available = false;
volatile char imu_data_updated = 0;
const uint32_t imu_speed[8] = {0, 4800, 9600, 19200, 38400, 57600, 115200, 230400};

MS5837 depth_sensor;


void initDepthSensor() {
  const int max_attempts = 3;
  int attempts = 0;

  while (attempts < max_attempts && !depth_sensor.init()) {
    uartDebug("❌ Depth sensor init failed");
    attempts++;
    delay(2000);
  }

  if (attempts < max_attempts) {
    uartDebug("✅ Depth sensor initialized");
    depth_sensor.setModel(MS5837::MS5837_30BA);
    depth_sensor.setFluidDensity(997); // freshwater; use 1029 for seawater
    depth_sensor_available = true;
  } else {
    uartDebug("⚠️ Depth sensor unavailable — running without it");
    depth_sensor_available = false;
  }
}

static void sensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
	Serial1.write(p_data, uiSize);
	Serial1.flush();
}

static void delayMs(uint16_t ucMs)
{
	delay(ucMs);
}

static void sensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
	for (i = 0; i < uiRegNum; i++)
	{
		switch (uiReg)
		{
		case AZ:
			imu_data_updated |= ACC_UPDATE;
			break;
		case GZ:
			imu_data_updated |= GYRO_UPDATE;
			break;
		case HZ:
			imu_data_updated |= MAG_UPDATE;
			break;
		case Yaw:
			imu_data_updated |= ANGLE_UPDATE;
			break;
		default:
			imu_data_updated |= READ_UPDATE;
			break;
		}
		uiReg++;
	}
}

static void autoScanSensor(void)
{
	int i, iRetry;

	for (i = 0; i < sizeof(imu_speed) / sizeof(imu_speed[0]); i++)
	{

		char buffer[100];
		sprintf(buffer, "ℹ️ Autoscan sensor on  %d  baud rate", imu_speed[i]);
		uartDebug(buffer);

		Serial1.begin(imu_speed[i], SERIAL_8N1, 27, 26);
		Serial1.flush();
		iRetry = 2;
		imu_data_updated = 0;

		do{
			int is_available = Serial1.available();

			if (is_available)
			{
				uartDebug("✅IMU serial available, try read data");
			}
			else
			{
				uartDebug("❌IMU serial not available, retry");
			}

			WitReadReg(AX, 3);
			delay(400);
			while (is_available)
			{
				char u = Serial1.read();

				WitSerialDataIn(u);

				is_available = Serial1.available();
			}

			if (imu_data_updated != 0)
			{
				char buffer[50];
				sprintf(buffer, "✅IMU find on %d baud rate speed", imu_speed[i]);
				uartDebug(buffer);

				imu_initialized = true;
				return;
			}
			iRetry--;
		} while (iRetry);
	}
	uartDebug("❌IMU autoscan failed");
}

void processThrusters(){

}

void initIMU(){
    uartDebug("ℹ️ Initialize IMU start");

	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(sensorUartSend);
	WitRegisterCallBack(sensorDataUpdata);
	WitDelayMsRegister(delayMs);
	autoScanSensor();

	uartDebug("ℹ️ Initialize IMU finish");
}

bool print_once = true;

void sensorTask(void *param)
{
	uartDebug("ℹ️ Sensor task started");

	while (true)
	{
		unsigned long currentMillis = millis();
		vTaskDelay(pdMS_TO_TICKS(5));

		if (imu_initialized == true)
		{
			while (Serial1.available())
			{
				WitSerialDataIn(Serial1.read());
			}
		} else {
			if (print_once){
				print_once = false;
				uartDebug("❌IMU not initialized, please check connection");
			}
		}
		if (depth_sensor_available == true){
			depth_sensor.read();
		}

	}
}

