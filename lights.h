#ifndef LIGHTS_H
#define LIGHTS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "color.h"

class Lights {
public:
  Lights();
  void setPixel(int idx, Color color);
  void begin();
  void clear();
  void show();
  void setBrightness(uint8_t brightness);

protected:
  Adafruit_NeoPixel	strip;
};

#endif
