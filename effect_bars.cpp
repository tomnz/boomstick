#include "effect_bars.h"

EffectBars::EffectBars() {
  bgColorBase = CRGB(0, 0, 0);
  bgColorBase.setHue(BACKGROUND_COLOR);
}

void EffectBars::loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {
  uint8_t clippedLevel, i;

  int barLevel = (int)(transformedLevel * (double)BAR_TOP * (double)BAR_SCALE);

  // Clip output and convert to byte:
  if (barLevel < 0) {
    clippedLevel = 0;
  }
  else if (barLevel > BAR_TOP) {
    clippedLevel = BAR_TOP; // Allow dot to go a couple pixels off top
  }
  else {
    clippedLevel = (uint8_t)barLevel;
  }

  if (clippedLevel > peak) {
    peak = clippedLevel; // Keep dot on top
  }

  if (barLevel <= BACKGROUND_CUTOFF && bgLevel < BACKGROUND_MAX) {
    bgLevel += BACKGROUND_INCREASE;
  }
  else if (barLevel > BACKGROUND_CUTOFF && bgLevel > 0) {
    bgLevel -= BACKGROUND_DECREASE;
  }
  bgLevel = max(0, min(bgLevel, BACKGROUND_MAX));

  CRGB bgColor;
  if (bgLevel > 0) {
    bgColor = CRGB(bgColorBase);
    bgColor.nscale8_video((int)(255.0 * bgLevel));
  }
  else {
    bgColor = CRGB(0, 0, 0);
  }

  int volumeEffect = (((BAR_COL_RANGE - BAR_COL_VAR)/2) * (double)smoothedLevel * HISTORIC_SCALE / historicLevel) + (BAR_COL_VAR/2);

  if (smoothedLevel < historicLevel * 0.4) {
    barLevel = 0;
  }

  if (volumeEffect > (BAR_COL_RANGE - BAR_COL_VAR/2)) {
    volumeEffect = (BAR_COL_RANGE - BAR_COL_VAR/2);
  }
  if (volumeEffect < (BAR_COL_VAR / 2)) {
    volumeEffect = (BAR_COL_VAR / 2);
  }

  // Color pixels based on rainbow gradient
  for (i = 0; i < N_PIXELS; i++) {
    if (i >= barLevel) {
      lights->setPixel(i, bgColor);
    }
    else {
      uint8_t pixelHue = volumeEffect + map(i, 0, N_PIXELS - 1, 0, BAR_COL_VAR) + BAR_COL_LOW;
#ifdef BAR_COL_INVERT
      pixelHue = BAR_COL_HIGH - pixelHue + BAR_COL_LOW;
#endif

      lights->setPixel(i, CHSV(pixelHue, 255, 255));
    }
  }

  // Draw peak dot
  if (peak > 2) {
    uint8_t pixelHue = volumeEffect + map(peak, 0, N_PIXELS - 1, 0, BAR_COL_VAR) + BAR_COL_LOW;
#ifdef BAR_COL_INVERT
    pixelHue = BAR_COL_HIGH - pixelHue + BAR_COL_LOW;
#endif
    CRGB color = CHSV(pixelHue, 255, 255);

    uint8_t peakI = (uint8_t)peak;

    for (short offset = -PEAK_RADIUS; offset <= PEAK_RADIUS; offset++) {
      i = peakI + offset;
      if (i <= barLevel || i >= N_PIXELS) continue;

      fract8 intensity = 255 / (abs(offset) + 1);
      lights->setPixel(i, bgColor.lerp8(color, intensity));
    }
  }

  // Drop the peak by its fall rate
  peak -= PEAK_FALL_RATE;
}
