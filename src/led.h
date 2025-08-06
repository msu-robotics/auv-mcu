#pragma once

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LedIndicator {
public:
    enum class EffectType {
        STATIC,
        FADE,
        TRANSITION
    };

    LedIndicator(uint16_t numPixels, uint8_t pin, uint8_t brightness = 50);

    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void fade(uint8_t r, uint8_t g, uint8_t b, uint16_t durationMs = 1000);
    void transitionTo(uint8_t r, uint8_t g, uint8_t b, uint16_t durationMs = 1000);
    void update();

private:
    Adafruit_NeoPixel strip;
    EffectType currentEffect;

    uint8_t currentR = 0, currentG = 0, currentB = 0;
    uint8_t targetR = 0, targetG = 0, targetB = 0;
    uint8_t startR = 0, startG = 0, startB = 0;

    unsigned long effectStart = 0;
    unsigned long fadeDuration = 1000;
    unsigned long transitionDuration = 1000;

    void applyColor(uint8_t r, uint8_t g, uint8_t b);
    uint8_t interpolate(uint8_t start, uint8_t end, float t);
};
