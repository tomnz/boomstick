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
  void begin();
  void clear();
  void show();
  void setBrightness(uint8_t brightness);

protected:
  CRGBArray<N_PIXELS> leds;
  
#ifdef MIRROR_DUPE
  CRGBArray<N_PIXELS*2> strip;
#else
  CRGBArray<N_PIXELS> strip;
#endif
};

#endif
