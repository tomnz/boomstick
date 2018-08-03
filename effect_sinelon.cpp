#include "effect_sinelon.h"

EffectSinelon::EffectSinelon() {
#ifdef SINELON_MIRROR
  mirror = true;
  numPixels /= 2;
  lastPixel = numPixels - 1;
#endif

  for (uint8_t i = 0; i < SINELON_DOTS; i++) {
    dots[i].init(
      SINELON_POS_RATE,
      SINELON_POS_MIN_LEVEL,
      SINELON_PERIOD_MIN,
      SINELON_PERIOD_MAX,
      SINELON_NUM_OSCILLATORS,
      lastPixel
    );
  }
}

void EffectSinelon::loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {
  transformedLevel -= SINELON_LEVEL_MIN;
  transformedLevel *= SINELON_LEVEL_SCALE;
  if (transformedLevel > level) {
    level = min(transformedLevel, 1.0);
  } else {
    level -= NOISE_LEVEL_DECAY;
  }
  level = max(0.0, level);

  hue += uint8_t(max(level, SINELON_HUE_MIN_LEVEL) * SINELON_HUE_RATE);

  lights->pixels().fadeToBlackBy(1 + uint8_t(level * 9));

  uint8_t pixel1, pixel2;
  uint8_t level8 = (uint8_t)(level * 255.0);
  CRGBSet *leds;
  for (uint8_t i = 0; i < SINELON_DOTS; i++) {
    dots[i].step(level);
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

void SinelonDot::init(
  double posMultiplier,
  double posMinLevel,
  uint8_t minPeriod,
  uint8_t maxPeriod,
  uint8_t numOscillators,
  uint8_t lastPixel
) {
  this->posMultiplier = posMultiplier;
  this->posMinLevel = posMinLevel;
  for (uint8_t i = 0; i < numOscillators; i++) {
    periods[i] = random8(minPeriod, maxPeriod);
  }
  this->numOscillators = numOscillators;
  this->lastPixel = lastPixel;
}

void SinelonDot::step(double level) {
  pos += uint32_t(max(level, posMinLevel) * posMultiplier);

  uint32_t avg = 0;
  for (uint8_t i = 0; i < numOscillators; i++) {
    avg += sin8(uint8_t(pos / periods[i]));
  }
  avg /= numOscillators;

  oldPixel = pixel;
  pixel = map(avg, 0, 255, 0, lastPixel);
}
