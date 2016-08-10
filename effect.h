#ifndef EFFECT_H
#define EFFECT_H

#include "lights.h"

class Effect {
public:
  virtual void loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {}
};

#endif
