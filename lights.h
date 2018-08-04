#ifndef LIGHTS_H
#define LIGHTS_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "color.h"

class Lights {
public:
  Lights();
  void setPixel(int idx, CRGB color);
  CRGB pixel(int idx);
  CRGBSet pixels();
  void clear();
  void show(bool mirror);
  void setBrightness(uint8_t brightness);

protected:
  CRGBArray<N_PIXELS> leds;
};

#endif
