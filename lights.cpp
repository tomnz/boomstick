#include "lights.h"

Lights::Lights() {
	// TODO: Support max refresh rate and wattage config?
	FastLED.addLeds<NEOPIXEL, LED_PIN>(strip, strip.size())
		.setCorrection(TypicalLEDStrip);
}

void Lights::setPixel(int idx, CRGB color) {
	if (idx < 0 || idx >= N_PIXELS) {
		return;
	}

#ifdef FLIP
	idx = N_PIXELS - idx - 1;
#endif

	strip[idx] = color;
}

void Lights::clear() {
	FastLED.clearData();
}

void Lights::show() {
#ifdef MIRROR_DUPE
	strip(N_PIXELS * 2 - 1, N_PIXELS) = strip(0, N_PIXELS - 1);
#endif

	FastLED.show();
}

void Lights::setBrightness(uint8_t brightness) {
	FastLED.setBrightness(brightness);
}
