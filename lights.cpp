#include "lights.h"

Lights::Lights() {
	// TODO: Support max refresh rate and wattage config?
	FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, leds.size())
		.setCorrection(TypicalLEDStrip);
}

void Lights::setPixel(int idx, CRGB color) {
	if (idx < 0 || idx >= N_PIXELS) return;
	leds[idx] = color;
}

CRGB Lights::pixel(int idx) {
	return leds[idx];
}

CRGBSet Lights::pixels() {
	return leds;
}

void Lights::clear() {
	leds.fill_solid(CRGB::Black);
}

void Lights::show(bool mirror) {
	if (mirror) {
		leds(N_PIXELS - 1, N_PIXELS/2) = leds(0, N_PIXELS/2 - 1);
	}
	FastLED.show();
}

void Lights::setBrightness(uint8_t brightness) {
	FastLED.setBrightness(brightness);
}
