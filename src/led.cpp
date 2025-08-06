#include "led.h"
#include <math.h>  // Для cos()


LedIndicator::LedIndicator(uint16_t numPixels, uint8_t pin, uint8_t brightness)
    : strip(numPixels, pin, NEO_GRB + NEO_KHZ800),
      currentEffect(EffectType::STATIC) {
    strip.begin();
    strip.setBrightness(brightness);
    strip.show();
}

void LedIndicator::setColor(uint8_t r, uint8_t g, uint8_t b) {
    targetR = r;
    targetG = g;
    targetB = b;
    currentEffect = EffectType::STATIC;
    applyColor(r, g, b);
}

void LedIndicator::fade(uint8_t r, uint8_t g, uint8_t b, uint16_t durationMs) {
    targetR = r;
    targetG = g;
    targetB = b;
    fadeDuration = durationMs;
    effectStart = millis();
    currentEffect = EffectType::FADE;
}

void LedIndicator::transitionTo(uint8_t r, uint8_t g, uint8_t b, uint16_t durationMs) {
    startR = currentR;
    startG = currentG;
    startB = currentB;
    targetR = r;
    targetG = g;
    targetB = b;
    transitionDuration = durationMs;
    effectStart = millis();
    currentEffect = EffectType::TRANSITION;
}

void LedIndicator::update() {
    unsigned long now = millis();

    switch (currentEffect) {
        case EffectType::STATIC:
            // Ничего не делаем
            break;

        case EffectType::FADE: {
            float t = (now - effectStart) / static_cast<float>(fadeDuration);
            if (t >= 1.0f) {
                effectStart = now;  // перезапускаем эффект (бесконечный fade)
                t = 0.0f;
            }

            float brightness = 0.5f * (1.0f - cosf(2.0f * PI * t));  // сглаженное синусом
            applyColor(
                static_cast<uint8_t>(targetR * brightness),
                static_cast<uint8_t>(targetG * brightness),
                static_cast<uint8_t>(targetB * brightness)
            );
            break;
        }

        case EffectType::TRANSITION: {
            float t = (now - effectStart) / static_cast<float>(transitionDuration);
            if (t >= 1.0f) {
                applyColor(targetR, targetG, targetB);
                currentEffect = EffectType::STATIC;
            } else {
                applyColor(
                    interpolate(startR, targetR, t),
                    interpolate(startG, targetG, t),
                    interpolate(startB, targetB, t)
                );
            }
            break;
        }
    }
}

void LedIndicator::applyColor(uint8_t r, uint8_t g, uint8_t b) {
    currentR = r;
    currentG = g;
    currentB = b;

    for (uint16_t i = 0; i < strip.numPixels(); ++i) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

uint8_t LedIndicator::interpolate(uint8_t start, uint8_t end, float t) {
    return static_cast<uint8_t>(start + (end - start) * t);
}
