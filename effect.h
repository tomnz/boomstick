#ifndef EFFECT_H
#define EFFECT_H

#include "lights.h"

class Effect {
public:
  virtual void loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {}
  bool mirror;
  uint8_t numPixels = N_PIXELS;
  uint8_t lastPixel = N_PIXELS - 1;
};

#endif
