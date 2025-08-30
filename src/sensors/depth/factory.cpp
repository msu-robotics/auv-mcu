#include "factory.h"
#include "utils.h"


ModularDepthSensor* createDepthSensor() {
    auto sensor = new ModularDepthSensor(nullptr, nullptr);

    #if defined(DEPTH_BMP180)
        Serial.println("ℹ️ Using BMP180 depth driver");
        auto unit = new BMP180Provider();
        sensor->setDepthProvider(static_cast<IDepthProvider*>(unit));
        auto temp = new BMP180Provider();
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(temp));
    #elif defined(DEPTH_MS5837)
        Serial.println("ℹ️ Using MS5837 depth driver");
        auto unit = new MS5837Provider();
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(unit));
        auto temp = new MS5837Provider();
        sensor->setDepthProvider(static_cast<IDepthProvider*>(temp));
    #else
        Serial.println("❌DEPTH driver not specified");
    #endif

    return sensor;
}
