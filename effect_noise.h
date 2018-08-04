#ifndef EFFECTNOISE_H
#define EFFECTNOISE_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "effect.h"
#include "lights.h"

class EffectNoise : public Effect {
public:
  EffectNoise();
  void loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  uint8_t x[N_PIXELS];
  uint8_t y[N_PIXELS];
  uint32_t realZ;
};

#endif
