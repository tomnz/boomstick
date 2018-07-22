#include "effect_noise.h"

// Loosely based on the 3rd example at:
// https://gist.github.com/StefanPetrick/5e853bea959e738bc6c2c2026683e3a4

EffectNoise::EffectNoise() {
  // Precalculate lookup tables - circular in the noise field so that each end
  // of the string matches up together
  for (uint16_t i = 0; i < N_PIXELS; i++) {
    uint8_t angle = (i * 256) / N_PIXELS;
    x[i] = cos8(angle);
    y[i] = sin8(angle);
  }
}

void EffectNoise::loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {
  double level = min(transformedLevel, 1.0);

  // Larger scale pushes the circle wider into the noise field - variation across
  // the length of the strip is greater as a result. Small scales result in a few
  // colors, with long smooth transitions between.
  uint32_t scale = NOISE_SCALE_BASE + level * NOISE_SCALE_DELTA;

  // Larger shift deltas result in travelling faster through the noise field, and
  // quicker color changes as a result.
  int zShift = max(NOISE_SHIFT_MIN, level * NOISE_SHIFT_FACTOR);
  realZ += zShift;


  for (uint16_t i = 0; i < N_PIXELS; i++) {
    // Calculate the coordinates within the noise field based on
    // the precalculated positions (no x/y shift)
    uint32_t realX = x[i] * scale;
    uint32_t realY = y[i] * scale;
    
    uint8_t noise = inoise16(realX, realY, realZ) >> 8;

    uint8_t hue = noise * 3;
    uint8_t bri = noise;

    CRGB color = CHSV(hue, 255, bri);
    lights->setPixel(i, color);
  }

  // Brightness - require real peaks to boost this (0.4 subtraction)
  lights->pixels().nscale8_video(NOISE_BRIGHTNESS_MIN + (uint8_t)(min(transformedLevel - 0.4, 1.0) * (255 - NOISE_BRIGHTNESS_MIN)));
}
