#pragma once
#include "sensors/imu/modular.h"
#include "sensors/providers/mpu6050.h"
#include "sensors/providers/hwt905.h"
#include "sensors/providers/qmc5883l.h"

ModularIMU* createIMU();