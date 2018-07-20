#ifndef EFFECTPULSE_H
#define EFFECTPULSE_H

#include "config.h"
#include "effect.h"
#include "lights.h"
#include "color.h"

class EffectPulse : public Effect {
public:
  EffectPulse();
  void loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  void chooseNewColor();

  Color currentColor;
  uint8_t currentColorPos;
  bool beatOn;
  uint16_t lowFrames;

  float brightness[N_PIXELS];
};

#endif
