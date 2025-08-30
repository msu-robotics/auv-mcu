#include "factory.h"
#include "sensors/common/temperatureProvider.h"
#include "utils.h"


ModularDepthSensor* createDepthSensor() {
    auto sensor = new ModularDepthSensor(nullptr, nullptr);

    #if defined(DEPTH_BMP180)
        Serial.println("ℹ️ Using BMP180 depth driver");
        auto unit = new BMP180Provider();
        sensor->setDepthProvider(static_cast<IDepthProvider*>(unit));
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(unit));
    #elif defined(DEPTH_MS5837)
        Serial.println("ℹ️ Using MS5837 depth driver");
        auto unit = new MS5837Provider();
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(unit));
        sensor->setDepthProvider(static_cast<IDepthProvider*>(unit));
    #else
        Serial.println("❌DEPTH driver not specified");
    #endif

    return sensor;
}
