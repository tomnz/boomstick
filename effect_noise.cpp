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
  transformedLevel -= NOISE_LEVEL_MIN;
  transformedLevel *= NOISE_LEVEL_SCALE;
  if (transformedLevel > level) {
    level = min(transformedLevel, 1.0);
  } else {
    level -= NOISE_LEVEL_DECAY;
  }
  level = max(0.0, level);

  // Larger scale pushes the circle wider into the noise field - variation across
  // the length of the strip is greater as a result. Small scales result in a few
  // colors, with long smooth transitions between.
  uint32_t scale = NOISE_SCALE_BASE + level * NOISE_SCALE_DELTA;

  // Larger shift deltas result in travelling faster through the noise field, and
  // quicker color changes as a result.
  int zShift = max(NOISE_SHIFT_MIN, level * NOISE_SHIFT_FACTOR);
  realZ += zShift;

  uint8_t maxBrightness = map((uint8_t)(level * 255.0), 0, 255, NOISE_BRIGHTNESS_MIN, 255);
  uint8_t saturation = map((uint8_t)(level * 255.0), 0, 255, NOISE_SATURATION_MIN, 255);

  for (uint16_t i = 0; i < N_PIXELS; i++) {
    // Calculate the coordinates within the noise field based on
    // the precalculated positions (no x/y shift)
    uint32_t realX = x[i] * scale;
    uint32_t realY = y[i] * scale;
    
    uint8_t noise = inoise16(realX, realY, realZ) >> 8;

    uint8_t hue = noise * 3;

    // TODO: This brightness/contrast shift is probably really inefficient and should
    // be revisited! Works for now...
    uint8_t bri = map(noise, 0, 255, 0, maxBrightness);

    // Apply contrast around midpoint, based on level (0 level = 0 contrast added)
    double diff = level * (bri - NOISE_CONTRAST_MIDPOINT) * NOISE_CONTRAST_AMOUNT;

    bri = max(min(bri + diff, 255), 0);

    CRGB color = CHSV(hue, saturation, bri);
    lights->setPixel(i, color);
  }
}
