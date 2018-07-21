#include "lights.h"

Lights::Lights() {
	// For mirrored strings, we need to double the number of pixels
	#ifdef MIRROR_DUPE
	size = N_PIXELS * 2;
	#else
	size = N_PIXELS;
	#endif
	
	strip = new CRGB[size];

	// TODO: Support max refresh rate and wattage config?
	FastLED.addLeds<NEOPIXEL, LED_PIN>(strip, size)
		.setCorrection(TypicalLEDStrip);
}

void Lights::setPixel(int idx, CRGB color) {
#ifdef FLIP
	idx = N_PIXELS - idx - 1;
#endif

	strip[idx] = color;

#ifdef MIRROR_DUPE
	strip[N_PIXELS * 2 - idx - 1] = color;
#endif
}

void Lights::clear() {
	FastLED.clearData();
}

void Lights::show() {
	FastLED.show();
}

void Lights::setBrightness(uint8_t brightness) {
	FastLED.setBrightness(brightness);
}
