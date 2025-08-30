#pragma once

#include "sensors/depth/modular.h"
#include <Wire.h>
#include "utils.h"
#include <MS5837.h>


#define TEMP_IS_ACTUAL   (1 << 0)
#define DEPTH_IS_ACTUAL  (1 << 1)


class MS5837Provider : public IDepthProvider, public ITemperatureProvider {
private:
    MS5837 sensor;
    int sda, scl;
    bool initialized = false;
    uint8_t data_is_actual = 0;
    unsigned long lastReadTime = 0;
    const unsigned long READ_INTERVAL = 100; // минимальный интервал между чтениями (мс)

    // Мьютекс для защиты I2C операций
    static SemaphoreHandle_t i2cMutex;
    static MS5837Provider* _instance;
    static bool wireInitialized;

    // Безопасное чтение с повторными попытками
    bool safeRead() {
        if (!initialized) {
            uartDebug("❌ MS5837 not initialized");
            return false;
        }

        // Проверяем интервал между чтениями
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime < READ_INTERVAL) {
            return true; // используем предыдущие данные
        }

        // Получаем мьютекс с таймаутом
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
            uartDebug("❌ Failed to acquire I2C mutex");
            return false;
        }

        bool success = false;
        int attempts = 3;

        while (attempts-- > 0 && !success) {
            try {
                // MS5837::read() возвращает void, поэтому просто вызываем
                sensor.read();

                // Проверяем валидность полученных данных
                float temp = sensor.temperature();
                float pressure = sensor.pressure();

                // Если значения в разумных пределах, считаем чтение успешным
                if (temp > -50.0f && temp < 100.0f && pressure > 0 && pressure < 20000.0f) {
                    success = true;
                    lastReadTime = currentTime;
                    data_is_actual = TEMP_IS_ACTUAL | DEPTH_IS_ACTUAL;
                } else {
                    uartDebug("❌ MS5837 read returned invalid values");
                    delay(50);
                }
            } catch (...) {
                uartDebug("❌ MS5837 read exception, retrying...");
                delay(50);
            }
        }

        xSemaphoreGive(i2cMutex);

        if (!success) {
            uartDebug("❌ MS5837 read failed after all attempts");
        }

        return success;
    }

public:
    MS5837Provider(int sda_pin = 21, int scl_pin = 22)
        : sda(sda_pin), scl(scl_pin) {
        _instance = this;

        // Создаем мьютекс если его еще нет
        if (i2cMutex == NULL) {
            i2cMutex = xSemaphoreCreateMutex();
            if (i2cMutex == NULL) {
                uartDebug("❌ Failed to create I2C mutex");
            }
        }
    }

    bool begin() override {
        if (initialized) {
            return true;
        }

        char buffer[100];
        sprintf(buffer, "ℹ️ MS5837 initialize sda: %d scl: %d", sda, scl);
        uartDebug(buffer);

        // инициализируем Wire если еще не инициализирован
        if (!wireInitialized) {
            uartDebug("🔧 Initializing Wire...");

            // Завершаем предыдущие операции
            Wire.end();
            delay(100);

            // Инициализируем с явным указанием пинов
            bool wireSuccess = Wire.begin(sda, scl);
            if (!wireSuccess) {
                uartDebug("❌ Wire.begin() failed!");
                return false;
            }

            // Устанавливаем разумную частоту
            Wire.setClock(100000); // 100kHz
            Wire.setTimeout(1000);  // 1 секунда таймаут

            wireInitialized = true;
            uartDebug("✅ Wire initialized");
        }

        // Получаем мьютекс для инициализации
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(2000)) != pdTRUE) {
            uartDebug("❌ Failed to acquire mutex for initialization");
            return false;
        }

        const int max_attempts = 5;
        int attempts = 0;
        bool initSuccess = false;

        while (attempts < max_attempts && !initSuccess) {
            attempts++;
            sprintf(buffer, "🔄 MS5837 init attempt %d/%d", attempts, max_attempts);
            uartDebug(buffer);

            if (sensor.init()) {
                initSuccess = true;
                uartDebug("✅ MS5837 sensor.init() successful");
            } else {
                sprintf(buffer, "❌ MS5837 init attempt %d failed", attempts);
                uartDebug(buffer);
                delay(1000); // задержка между попытками
            }
        }

        xSemaphoreGive(i2cMutex);

        if (initSuccess) {
            uartDebug("🔧 Setting MS5837 parameters...");
            sensor.setModel(MS5837::MS5837_30BA);
            sensor.setFluidDensity(997); // пресная вода; 1029 для морской
            initialized = true;
            uartDebug("✅ MS5837 fully initialized");

            // Выполняем первое чтение для проверки
            delay(100); // даем датчику время на готовность
            if (safeRead()) {
                sprintf(buffer, "📊 Initial reading - Temp: %.2f°C, Depth: %.2fm",
                       sensor.temperature(), sensor.depth());
                uartDebug(buffer);
            } else {
                uartDebug("⚠️ Initial reading failed, but continuing...");
            }

            return true;
        } else {
            uartDebug("❌ MS5837 initialization failed completely");
            initialized = false;
            return false;
        }
    }

    float readTemperature() override {
        if (!initialized) {
            uartDebug("❌ MS5837 not initialized for temperature reading");
            return -999.0f; // код ошибки
        }

        // Обновляем данные если они не актуальны
        if (!(data_is_actual & TEMP_IS_ACTUAL)) {
            if (!safeRead()) {
                uartDebug("❌ Failed to update temperature data");
                return -999.0f;
            }
        }

        data_is_actual &= ~TEMP_IS_ACTUAL;
        float temp = sensor.temperature();

        // Проверяем разумность значения
        if (temp < -50.0f || temp > 100.0f) {
            uartDebug("⚠️ Temperature reading out of range");
            return -999.0f;
        }

        return temp;
    }

    float readDepth() override {
        if (!initialized) {
            uartDebug("❌ MS5837 not initialized for depth reading");
            return -999.0f;
        }

        // Обновляем данные если они не актуальны
        if (!(data_is_actual & DEPTH_IS_ACTUAL)) {
            if (!safeRead()) {
                uartDebug("❌ Failed to update depth data");
                return -999.0f;
            }
        }

        data_is_actual &= ~DEPTH_IS_ACTUAL;
        float depth = sensor.depth();

        // Проверяем разумность значения (предполагаем максимальную глубину 1000м)
        if (depth < -10.0f || depth > 1000.0f) {
            char buffer[50];
            sprintf(buffer, "⚠️ Depth reading out of range: %.2f", depth);
            uartDebug(buffer);
            return -999.0f;
        }

        return depth;
    }

    // Статус датчика
    bool isInitialized() const {
        return initialized;
    }

    // Принудительное обновление данных
    bool forceUpdate() {
        data_is_actual = 0; // сбрасываем флаги актуальности
        return safeRead();
    }

    // Переинициализация датчика
    bool reinitialize() {
        uartDebug("🔄 Reinitializing MS5837...");
        initialized = false;
        data_is_actual = 0;
        delay(100);
        return begin();
    }

    // Диагностика I2C соединения
    bool testConnection() {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
            return false;
        }

        Wire.beginTransmission(0x76); // Адрес MS5837 по умолчанию
        uint8_t error = Wire.endTransmission();

        xSemaphoreGive(i2cMutex);

        if (error == 0) {
            uartDebug("✅ MS5837 I2C connection OK");
            return true;
        } else {
            char buffer[50];
            sprintf(buffer, "❌ MS5837 I2C error: %d", error);
            uartDebug(buffer);
            return false;
        }
    }

    ~MS5837Provider() {
        if (i2cMutex != NULL) {
            vSemaphoreDelete(i2cMutex);
            i2cMutex = NULL;
        }
    }
};
