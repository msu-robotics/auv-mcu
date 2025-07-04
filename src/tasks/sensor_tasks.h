#ifndef SENSOR_TASK
#define SENSOR_TASK
#pragma once

#include <REG.h>
#include <wit_c_sdk.h>
#include "MS5837.h"

void sensorTask(void *param);
void initDepthSensor();
void initIMU();

extern volatile char imu_data_updated;
extern bool imu_initialized;

extern MS5837 depth_sensor;

#endif