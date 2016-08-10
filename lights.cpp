#include "lights.h"

Lights::Lights() {
	// For mirrored strings, we need to double the number of pixels
	#ifdef MIRROR_DUPE
	strip = Adafruit_NeoPixel(N_PIXELS * 2, LED_PIN, NEO_GRB + NEO_KHZ800);
	#else
	strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
	#endif
}

void Lights::setPixel(int idx, Color color) {
#ifdef FLIP
    idx = N_PIXELS - idx - 1;
#endif

  strip.setPixelColor(idx, color.r, color.g, color.b);

#ifdef MIRROR_DUPE
  strip.setPixelColor(N_PIXELS * 2 - idx - 1, color.r, color.g, color.b);
#endif
}

void Lights::begin() {
	strip.begin();
}

void Lights::clear() {
	strip.clear();
}

void Lights::show() {
	strip.show();
}

void Lights::setBrightness(uint8_t brightness) {
	strip.setBrightness(brightness);
}
