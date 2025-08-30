#include "factory.h"
#include "utils.h"


ModularPressureSensor* createPressureSensor() {
    auto sensor = new ModularPressureSensor(nullptr, nullptr);

    #if defined(PRESS_BMP180)
        Serial.println("ℹ️ Using BMP180 pressure driver");
        auto unit = new BMP180Provider();
        sensor->setPressureProvider(static_cast<IPressureProvider*>(unit));
        auto temp = new BMP180Provider();
        sensor->setTemperatureProvider(static_cast<ITemperatureProvider*>(temp));

    #else
        Serial.println("❌PRESS driver not specified");
    #endif

    return sensor;
}
