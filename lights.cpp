#include "lights.h"

Lights::Lights() {
	// TODO: Support max refresh rate and wattage config?
	FastLED.addLeds<NEOPIXEL, LED_PIN>(strip, strip.size())
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
	#ifdef FLIP
		strip(N_PIXELS/2 - 1, 0) = leds(0, N_PIXELS/2 - 1);
		strip(N_PIXELS/2, N_PIXELS - 1) = leds(0, N_PIXELS/2 - 1);
	#else
		strip(0, N_PIXELS/2 - 1) = leds(0, N_PIXELS/2 - 1);
		strip(N_PIXELS - 1, N_PIXELS/2) = leds(0, N_PIXELS/2 - 1);
	#endif
	} else {
	#ifdef FLIP
		strip(N_PIXELS - 1, 0) = leds;
	#else
		strip(0, N_PIXELS - 1) = leds;
	#endif
	}

	FastLED.show();
}

void Lights::setBrightness(uint8_t brightness) {
	FastLED.setBrightness(brightness);
}
