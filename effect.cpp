#include "effect.h"

Effect::Effect(bool mirror) : mirror(mirror) {
  if (mirror) {
    numPixels = N_PIXELS / 2;
  } else {
    numPixels = N_PIXELS;
  }
  lastPixel = numPixels - 1;
}
