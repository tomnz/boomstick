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
  void init(
    double posMultiplier,
    double posMinLevel,
    uint8_t minPeriod,
    uint8_t maxPeriod,
    uint8_t numOscillators,
    uint8_t lastPixel
  );
  void step(double level);
  uint8_t oldPixel;
  uint8_t pixel;

protected:
  double posMultiplier;
  double posMinLevel;
  uint32_t pos;
  uint8_t periods[5];
  uint8_t numOscillators;
  uint8_t lastPixel;
};


class EffectSinelon : public Effect {
public:
  EffectSinelon();
  void loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel);

protected:
  SinelonDot dots[SINELON_DOTS];
  uint8_t hue;
  double level;
};

#endif
