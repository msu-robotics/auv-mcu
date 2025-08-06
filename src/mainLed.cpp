#include <Arduino.h>
#include <FastLED.h>
#define NUM_LEDS 10
#define DATA_PIN 12
CRGB leds[NUM_LEDS];
void setup() {
    Serial.begin(115200);
    FastLED.addLeds<WS2812, DATA_PIN>(leds, NUM_LEDS);
}
void loop() {
	leds[0] = CRGB::White; FastLED.show(); delay(30);
	leds[0] = CRGB::Black; FastLED.show(); delay(30);
}
