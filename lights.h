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
  void begin();
  void clear();
  void show();
  void setBrightness(uint8_t brightness);

protected:
  CRGB *strip;
  int size;
};

#endif
