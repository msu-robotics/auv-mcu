#include <Arduino.h>
#include <REG.h>
#include <wit_c_sdk.h>

#define LED_PIN 2

void uartDebug(const char *debug) {

	Serial2.println(debug);

}

void fillArray(float *in, float *out, int count, float scale = 1.0f) {
	for (int i = 0; i < count; ++i) {
		out[i] = in[i] * scale;
	}
}

void fillCovariance(double* cov, double value) {
	for (int i = 0; i < 9; ++i)
		cov[i] = (i % 4 == 0) ? value : 0.0;
}


class Blinker {
	int led_pin;
	int blink_delay_ms = 100;

	public:
	void init() {
		pinMode(led_pin, OUTPUT);
		digitalWrite(led_pin, LOW);
		initialized = true;
	}

	void blink(int times = 1) {
		if (!initialized){
			init();
		}

		for (int i = 0; i < times; i++)
		{
			delay(blink_delay_ms);
			digitalWrite(led_pin, !digitalRead(led_pin));
			delay(blink_delay_ms);
			digitalWrite(led_pin, !digitalRead(led_pin));
		}
	}

	private:
		bool initialized = false;
};

