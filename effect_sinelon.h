#ifndef EFFECTSINELON_H
#define EFFECTSINELON_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "effect.h"
#include "lights.h"

class SinelonDot {
public:
  SinelonDot();
  void step(uint32_t pos, uint8_t lastPixel);
  uint8_t oldPixel;
  uint8_t pixel;
  uint8_t periods[SINELON_NUM_OSCILLATORS];
};


class EffectSinelon : public Effect {
public:
  EffectSinelon();
  void loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  SinelonDot dots[SINELON_DOTS];
  uint32_t pos;
  uint8_t hue;
};

#endif
