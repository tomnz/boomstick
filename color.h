#ifndef COLOR_H
#define COLOR_H

#include <avr/pgmspace.h>

class Color {
public:
  Color() {};
  Color(int r, int g, int b) : r(r), g(g), b(b) {};
  int r, g, b;
};

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
Color Wheel(uint8_t wheelPos);

#endif
