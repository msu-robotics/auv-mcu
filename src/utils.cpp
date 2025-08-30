#include <Arduino.h>

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
