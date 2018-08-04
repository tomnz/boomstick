#ifndef EFFECTPULSE_H
#define EFFECTPULSE_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "effect.h"
#include "lights.h"

class EffectPulse : public Effect {
public:
  EffectPulse();
  void loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  void chooseNewColor();

  CRGB currentColor;
  uint8_t currentColorPos;
  bool beatOn;
  uint16_t lowFrames;

  float brightness[N_PIXELS];
};

#endif
