#include "ms5837.h"

SemaphoreHandle_t MS5837Provider::i2cMutex = NULL;
MS5837Provider* MS5837Provider::_instance = nullptr;
bool MS5837Provider::wireInitialized = false;