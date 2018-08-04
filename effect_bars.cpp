#include "effect_bars.h"

EffectBars::EffectBars() : Effect(BAR_MIRROR) {
  bgColorBase = CRGB(0, 0, 0);
  bgColorBase.setHue(BACKGROUND_COLOR);
}

void EffectBars::loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel) {
  uint8_t idx;

  int barLevel = (int)(transformedLevel * BAR_LEVEL_SCALE * (double)lastPixel);

  if (barLevel < 0) {
    barLevel = -1;
  }
  if (barLevel > lastPixel) {
    barLevel = lastPixel;
  }

  if (barLevel > peak) {
    peak = barLevel; // Keep dot on top
  }

  // Draw background
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

  // Ensure we get no bar at times - makes the output more lively
  if (smoothedLevel < historicLevel * 0.4) {
    barLevel = -1;
  }
  if (barLevel < lastPixel) {
    #ifdef BAR_FLIP
    lights->pixels()(0, lastPixel - barLevel - 1).fill_solid(bgColor);
    #else
    lights->pixels()(barLevel+1, lastPixel).fill_solid(bgColor);
    #endif
  }

  // Draw bar
  int volumeEffect = (((BAR_COL_RANGE - BAR_COL_VAR)/2) * (double)smoothedLevel * HISTORIC_SCALE / historicLevel) + (BAR_COL_VAR/2);

  if (volumeEffect > (BAR_COL_RANGE - BAR_COL_VAR/2)) {
    volumeEffect = (BAR_COL_RANGE - BAR_COL_VAR/2);
  }
  if (volumeEffect < (BAR_COL_VAR / 2)) {
    volumeEffect = (BAR_COL_VAR / 2);
  }

  if (barLevel >= 0) {
    uint8_t rainbowStart = volumeEffect + BAR_COL_LOW;
    uint8_t rainbowDelta = map(barLevel, 0, lastPixel, 0, BAR_COL_VAR) / barLevel;
  #ifdef BAR_COL_INVERT
    rainbowStart = BAR_COL_HIGH - rainbowStart + BAR_COL_LOW;
    rainbowDelta = -rainbowDelta;
  #endif

  #ifdef BAR_FLIP
    lights->pixels()(lastPixel, lastPixel - barLevel).fill_rainbow(rainbowStart, rainbowDelta);
  #else
    lights->pixels()(0, barLevel).fill_rainbow(rainbowStart, rainbowDelta);
  #endif
  }

  // Draw peak dot
  if (peak > 2) {
    uint8_t pixelHue = volumeEffect + map(peak, 0, lastPixel, 0, BAR_COL_VAR) + BAR_COL_LOW;
#ifdef BAR_COL_INVERT
    pixelHue = BAR_COL_HIGH - pixelHue + BAR_COL_LOW;
#endif
    CRGB color = CHSV(pixelHue, 255, 255);

    uint8_t peakI = (uint8_t)peak;

    for (short offset = -PEAK_RADIUS; offset <= PEAK_RADIUS; offset++) {
      idx = peakI + offset;
      if (idx <= barLevel || idx >= numPixels) continue;

      fract8 intensity = 255 / (abs(offset) + 1);
      #ifdef BAR_FLIP
      idx = lastPixel - idx;
      #endif
      lights->setPixel(idx, lights->pixel(idx).lerp8(color, intensity));
    }
  }

  // Drop the peak by its fall rate
  peak -= PEAK_FALL_RATE;
}
