#pragma once
#include <Arduino.h>

class Thruster {
public:
    Thruster(uint8_t pin, uint8_t channel,
             int neutral_us = 1500,
             int min_us = 1100, int max_us = 1900);

    void setup();
    void setPower(int8_t power);
    void writeMicroseconds(int us);

    void setDirectionReversed(bool reversed);
    bool isReversed() const;

private:
    uint8_t _pin;
    uint8_t _channel;
    int _neutral_us;
    int _min_us;
    int _max_us;
    bool _reversed = false;

    uint32_t microsToDuty(int us) const;
};
