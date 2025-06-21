#include <Arduino.h>

#define BAUD_RATE 9600

void setup()
{
	Serial.begin(BAUD_RATE);
	while (!Serial) {
		delay(10);
	}
	Serial1.begin(BAUD_RATE, SERIAL_8N1, 27, 26);
}

void loop()
{
	if (Serial1.available())
	{
		char c = Serial1.read();
		Serial.write(c);
	}
	if (Serial.available())
	{
		char c = Serial.read();
		Serial1.write(c);
	}
}
