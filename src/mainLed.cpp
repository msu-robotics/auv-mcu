#include <Arduino.h>
#include "led.h"

LedIndicator led(100, 12);

void setup() {
    led.setColor(0, 0, 255);
    delay(1000);
    led.fade(255, 0, 0, 2000);
}

void loop() {
    led.update();
}
