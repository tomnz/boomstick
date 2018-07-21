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

void Lights::clear() {
	leds.fill_solid(CRGB::Black);
}

void Lights::show() {
#ifdef MIRROR_DUPE
	#ifdef FLIP
		strip(N_PIXELS - 1, 0) = leds;
		strip(N_PIXELS, N_PIXELS * 2 - 1) = leds;
	#else
		strip(0, N_PIXELS - 1) = leds;
		strip(N_PIXELS * 2 - 1, N_PIXELS) = leds;
	#endif
#else
	#ifdef FLIP
		strip(N_PIXELS - 1, 0) = leds;
	#else
		strip(0, N_PIXELS - 1) = leds;
	#endif
#endif

	FastLED.show();
}

void Lights::setBrightness(uint8_t brightness) {
	FastLED.setBrightness(brightness);
}
