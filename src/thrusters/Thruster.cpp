#include "Thruster.h"

Thruster::Thruster(uint8_t pin, uint8_t channel, int neutral_us, int min_us, int max_us)
    : _pin(pin), _channel(channel),
      _neutral_us(neutral_us),
      _min_us(min_us), _max_us(max_us)
{}

void Thruster::setup() {
    ledcSetup(_channel, 50, 16);           // 50 Гц, 16 бит
    ledcAttachPin(_pin, _channel);
    writeMicroseconds(_neutral_us);        // арминг
    delay(2000);
}

void Thruster::setPower(int8_t power) {
    power = constrain(power, -100, 100);
    if (_reversed) power = -power;
    int us = _neutral_us + power * 5;      // 1% = 5us
    us = constrain(us, _min_us, _max_us);
    writeMicroseconds(us);
}

void Thruster::writeMicroseconds(int us) {
    us = constrain(us, _min_us, _max_us);
    ledcWrite(_channel, microsToDuty(us));
}

uint32_t Thruster::microsToDuty(int us) const {
    return (uint32_t)(((uint64_t)us * 65535) / 20000);  // 20 ms период
}

void Thruster::setDirectionReversed(bool reversed) {
    _reversed = reversed;
}

bool Thruster::isReversed() const {
    return _reversed;
}
