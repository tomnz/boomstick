#ifndef LIGHTS_H
#define LIGHTS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

class Color {
public:
  Color() {};
  Color(int r, int g, int b) : r(r), g(g), b(b) {};
  int r, g, b;
};

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
Color Wheel(byte wheelPos);

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
