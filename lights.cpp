#include "lights.h"

Lights::Lights() {
	// TODO: Support max refresh rate and wattage config?
	FastLED.addLeds<NEOPIXEL, LED_PIN>(strip, strip.size())
		.setCorrection(TypicalLEDStrip);
}

void Lights::setPixel(int idx, CRGB color) {
	if (idx < 0 || idx > LAST_PIXEL) return;
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

void Lights::show() {
#ifdef MIRROR_DUPE
	#ifdef FLIP
		strip(LAST_PIXEL, 0) = leds;
		strip(N_PIXELS, N_PIXELS * 2 - 1) = leds;
	#else
		strip(0, LAST_PIXEL) = leds;
		strip(N_PIXELS * 2 - 1, N_PIXELS) = leds;
	#endif
#else
	#ifdef FLIP
		strip(LAST_PIXEL, 0) = leds;
	#else
		strip(0, LAST_PIXEL) = leds;
	#endif
#endif

	FastLED.show();
}

void Lights::setBrightness(uint8_t brightness) {
	FastLED.setBrightness(brightness);
}
