#include "factory.h"
#include "utils.h"
#include "sensors/providers/adc.h"


ModularPowerSensor* createPowerSensor() {
    auto sensor = new ModularPowerSensor(nullptr, nullptr);

    #if defined(POWER_ADC)
        uartDebug("ℹ️ Using ADC power sensor driver");
        auto unit = new AdcProvider(2, 4);
        sensor->setCurrentProvider(static_cast<ICurrentProvider*>(unit));
        sensor->setVoltageProvider(static_cast<IVoltageProvider*>(unit));
    #else
        uartDebug("❌POWER sensor driver not specified");
    #endif

    return sensor;
}
