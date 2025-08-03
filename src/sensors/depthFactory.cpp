#include "depthFactory.h"
#include "utils.h"


ModularDepthSensor* createDepthSensor() {
    auto sensor = new ModularDepthSensor(nullptr, nullptr);

    #if defined(DEPTH_BMP180)
        Serial.println("ℹ️ Using BMP180 depth driver");
        auto unit = new BMP180Provider();
        sensor->setDepthProvider(static_cast<IDepthProvider*>(unit));
    #elif defined(DEPTH_MS5837)
        Serial.println("ℹ️ Using MS5837 depth driver");
        auto unit = new MS5837Provider();
        sensor->setDepthProvider(static_cast<IDepthProvider*>(unit));
    #else
        Serial.println("❌DEPTH driver not specified");
    #endif

    #if defined(TEMP_BMP180)
        Serial.println("ℹ️ Using BMP180 TEMPerature driver");
        auto temp = new BMP180Provider();
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(temp));
    #elif defined(TEMP_MS5837)
        Serial.println("ℹ️ Using MS5837 TEMPerature driver");
        auto temp = new MS5837Provider();
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(temp));
    #else
        Serial.println("❌ TEMPerature driver not specified");
    #endif

        if (!sensor->begin()) {
            Serial.println("❌ DEPTH init failed!");
            while (true);
        }

    return sensor;
}
