#include "factory.h"
#include "utils.h"


ModularPressureSensor* createPressureSensor() {
    auto sensor = new ModularPressureSensor(nullptr, nullptr);

    #if defined(PRESS_BMP280)
        Serial.println("ℹ️ Using BMP280 pressure driver");
        auto unit = new BME280Provider();
        sensor->setPressureProvider(static_cast<IPressureProvider*>(unit));
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(unit));

    #else
        Serial.println("❌PRESS driver not specified");
    #endif

    return sensor;
}
