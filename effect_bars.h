#ifndef EFFECTBARS_H
#define EFFECTBARS_H

#include "config.h"
#include "effect.h"
#include "lights.h"

class EffectBars : public Effect {
public:
  EffectBars();
  void loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  float peak, backgroundLevel;
};

#endif
