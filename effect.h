#ifndef EFFECT_H
#define EFFECT_H

#include "lights.h"

class Effect {
public:
  Effect();
  Effect(bool mirror);

  virtual void loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel) {}
  bool mirror;
  uint8_t numPixels, lastPixel;
};

#endif
