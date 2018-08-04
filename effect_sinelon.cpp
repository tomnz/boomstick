#include "effect_sinelon.h"

EffectSinelon::EffectSinelon() : Effect(SINELON_MIRROR) {
  for (uint8_t i = 0; i < SINELON_DOTS; i++) {
    for (uint8_t j = 0; j < SINELON_NUM_OSCILLATORS; j++) {
      dots[i].periods[j] = random8(SINELON_PERIOD_MIN, SINELON_PERIOD_MAX);
    }
  }
}

void EffectSinelon::loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel) {
  pos += uint32_t(max(sanitizedLevel, SINELON_POS_MIN_LEVEL) * SINELON_POS_RATE);
  hue += uint8_t(max(sanitizedLevel, SINELON_HUE_MIN_LEVEL) * SINELON_HUE_RATE);

  lights->pixels().fadeToBlackBy(1 + uint8_t(sanitizedLevel * 9));

  uint8_t pixel1, pixel2;
  uint8_t level8 = (uint8_t)(sanitizedLevel * 255.0);
  CRGBSet *leds;
  for (uint8_t i = 0; i < SINELON_DOTS; i++) {
    dots[i].step(pos, lastPixel);
    pixel1 = dots[i].oldPixel;
    pixel2 = dots[i].pixel;
    
    if (pixel1 <= pixel2) {
      leds = &lights->pixels()(pixel1, pixel2);
    } else {
      leds = &lights->pixels()(pixel2, pixel1);
    }

    leds->fill_solid(CHSV(hue, map(level8, 0, 255, SINELON_SATURATION_MIN, 255), map(level8, 0, 255, SINELON_BRIGHTNESS_MIN, 255)));
  }
}


SinelonDot::SinelonDot() {
}

void SinelonDot::step(uint32_t pos, uint8_t lastPixel) {
  uint32_t avg = 0;
  for (uint8_t i = 0; i < SINELON_NUM_OSCILLATORS; i++) {
    avg += sin8(uint8_t(pos / periods[i]));
  }
  avg /= SINELON_NUM_OSCILLATORS;

  oldPixel = pixel;
  pixel = map(avg, 0, 255, 0, lastPixel);
}
