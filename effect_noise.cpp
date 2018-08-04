#include "effect_noise.h"

// Loosely based on the 3rd example at:
// https://gist.github.com/StefanPetrick/5e853bea959e738bc6c2c2026683e3a4

EffectNoise::EffectNoise() : Effect(NOISE_MIRROR) {
#ifdef NOISE_CIRCULAR
  // Precalculate lookup tables - circular in the noise field so that each end
  // of the string matches up together
  for (uint16_t i = 0; i < numPixels; i++) {
    uint8_t angle = (i * 256) / numPixels;
    x[i] = cos8(angle);
    y[i] = sin8(angle);
  }
#else
  // Precalculate lookup tables - evenly space across x=[0, 255], but vary y
  for (uint16_t i = 0; i < numPixels; i++) {
    uint8_t angle = (i * 256) / numPixels;
    x[i] = angle;
    y[i] = sin8(angle);
  }
#endif
}

void EffectNoise::loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel) {
  // Larger scale pushes the circle wider into the noise field - variation across
  // the length of the strip is greater as a result. Small scales result in a few
  // colors, with long smooth transitions between.
  uint32_t scale = NOISE_SCALE_BASE + sanitizedLevel * NOISE_SCALE_DELTA;

  // Larger shift deltas result in travelling faster through the noise field, and
  // quicker color changes as a result.
  realZ += max(NOISE_SHIFT_MIN, sanitizedLevel * NOISE_SHIFT_FACTOR);

  uint8_t level8 = (uint8_t)(sanitizedLevel * 255.0);
  uint8_t maxBrightness = map(level8, 0, 255, NOISE_BRIGHTNESS_MIN, 255);
  uint8_t saturation = map(level8, 0, 255, NOISE_SATURATION_MIN, 255);

  uint32_t realX, realY;
  uint8_t noise, hue, bri;
  float diff;
  for (uint16_t i = 0; i < numPixels; i++) {
    // Calculate the coordinates within the noise field based on
    // the precalculated positions (no x/y shift)
    realX = x[i] * scale;
    realY = y[i] * scale;
    
    noise = inoise16(realX, realY, realZ) >> 8;

    hue = noise * 3;

    // TODO: This brightness/contrast shift is probably really inefficient and should
    // be revisited! Works for now...
    bri = map(noise, 0, 255, 0, maxBrightness);

    // Apply contrast around midpoint, based on level (0 level = 0 contrast added)
    diff = sanitizedLevel * (bri - NOISE_CONTRAST_MIDPOINT) * NOISE_CONTRAST_AMOUNT;
    bri = max(min(bri + diff, 255), 0);

    lights->setPixel(i, CHSV(hue, saturation, bri));
  }
}
