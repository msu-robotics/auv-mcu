#pragma once
#include "sensors/power/modular.h"
#include <Arduino.h>


class AdcProvider: public ICurrentProvider, public IVoltageProvider {
    int currentPin, voltagePin;
    float voltageCoeff = 147.6;
    float currentMult = 1.0;
    bool initialized = false;

    // Фильтрация
    static const int FILTER_SIZE = 10;
    float voltageBuffer[FILTER_SIZE];
    float currentBuffer[FILTER_SIZE];
    int bufferIndex = 0;
    bool bufferFilled = false;

public:
    AdcProvider(int currentPin, int voltagePin): currentPin(currentPin), voltagePin(voltagePin) {
        // Инициализируем буферы нулями
        for(int i = 0; i < FILTER_SIZE; i++) {
            voltageBuffer[i] = 0;
            currentBuffer[i] = 0;
        }
    }

    bool begin() override {
        // Увеличиваем разрешение до 12 бит (4096 значений)
        analogReadResolution(12);

        // Устанавливаем аттенюацию для нужного диапазона
        analogSetAttenuation(ADC_11db);  // 0-3.3V
        // или ADC_6db для 0-2.2V (выше точность)

        initialized = true;
        return initialized;
    }

    float readVoltage() override {
        float rawValue = readADCOversampled(voltagePin, 16) / voltageCoeff;

        voltageBuffer[bufferIndex] = rawValue;
        float sum = 0;
        int count = bufferFilled ? FILTER_SIZE : (bufferIndex + 1);

        for(int i = 0; i < count; i++) {
            sum += voltageBuffer[i];
        }

        updateBufferIndex();

        return sum / count;  // ← ДОБАВИТЬ return!
    }

    float readCurrent() override {
        float rawValue = currentMult * readADCOversampled(currentPin, 16);

        currentBuffer[bufferIndex] = rawValue;
        float sum = 0;
        int count = bufferFilled ? FILTER_SIZE : (bufferIndex + 1);

        for(int i = 0; i < count; i++) {
            sum += currentBuffer[i];
        }

        return sum / count;  // ← ДОБАВИТЬ return!
    }

private:
    void updateBufferIndex() {
        bufferIndex++;
        if(bufferIndex >= FILTER_SIZE) {
            bufferIndex = 0;
            bufferFilled = true;
        }
    }

    uint32_t readADCOversampled(int pin, int samples = 16) {
        uint32_t sum = 0;
        for(int i = 0; i < samples; i++) {
            sum += analogRead(pin);
            delayMicroseconds(100); // небольшая задержка между измерениями
        }
        return sum / samples;
    }
};