#ifndef EFFECTFIRE_H
#define EFFECTFIRE_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "effect.h"
#include "lights.h"


class EffectFire : public Effect {
public:
  EffectFire();
  void loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  byte heat[N_PIXELS];
};

#endif
