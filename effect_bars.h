#ifndef EFFECTBARS_H
#define EFFECTBARS_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "effect.h"
#include "lights.h"

class EffectBars : public Effect {
public:
  EffectBars();
  void loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  CRGB bgColorBase;
  float peak, bgLevel;
};

#endif
