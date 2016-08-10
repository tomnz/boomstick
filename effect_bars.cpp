#include "effect_bars.h"

EffectBars::EffectBars() {

}

void EffectBars::loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {
  Color color, backgroundColor;
  uint8_t clippedLevel, i;

  int barLevel = (int)(transformedLevel * (double)TOP);

  // Clip output and convert to byte:
  if (barLevel < 0L) {
    clippedLevel = 0;
  }
  else if (barLevel > TOP) {
    clippedLevel = TOP; // Allow dot to go a couple pixels off top
  }
  else {
    clippedLevel = (uint8_t)barLevel;
  }

  if (clippedLevel > peak) {
    peak = clippedLevel; // Keep dot on top
  }

  if (barLevel <= BACKGROUND_CUTOFF && backgroundLevel < BACKGROUND_MAX) {
    backgroundLevel += BACKGROUND_INCREASE;
  }
  else if (barLevel > BACKGROUND_CUTOFF && backgroundLevel > 0) {
    backgroundLevel -= BACKGROUND_DECREASE;
  }
  backgroundLevel = max(0, min(backgroundLevel, BACKGROUND_MAX));
  if (backgroundLevel > 0) {
    Color bg = Wheel(BACKGROUND_COLOR);
    backgroundColor = Color((float)bg.r * backgroundLevel, (float)bg.g * backgroundLevel, (float)bg.b * backgroundLevel);
  }
  else {
    backgroundColor = Color(0, 0, 0);
  }

  int volumeEffect = (((COL_RANGE - COL_VAR)/2) * (double)smoothedLevel * HISTORIC_SCALE / historicLevel) + (COL_VAR/2);

  if (smoothedLevel < historicLevel * 0.4) {
    barLevel = 0;
  }

  if (volumeEffect > (COL_RANGE - COL_VAR/2)) {
    volumeEffect = (COL_RANGE - COL_VAR/2);
  }
  if (volumeEffect < (COL_VAR / 2)) {
    volumeEffect = (COL_VAR / 2);
  }

  // Color pixels based on rainbow gradient
  lights->clear();

  for (i = 0; i < N_PIXELS; i++) {
    if (i >= barLevel) {
      lights->setPixel(i, backgroundColor);
    }
    else {
      color = Wheel(volumeEffect + map(i, 0, N_PIXELS - 1, 0, COL_VAR) + MIN_COL);
      lights->setPixel(i, color);
    }
  }

  // Draw peak dot
  if (peak > 2) {
    color = Wheel(volumeEffect + map(peak, 0, N_PIXELS - 1, 0, COL_VAR) + MIN_COL);
    for (i = max((int)peak - 2, barLevel); i < min((int)peak + 2, N_PIXELS - 1); i++) {
      float intensity = 1.0 - min(abs((float)i - peak) / 1.5, 1.0);
      Color peakColor = Color((int)((float)color.r * intensity), (int)((float)color.g * intensity), (int)((float)color.b * intensity));
      lights->setPixel(i, peakColor);
    }
  }

  lights->show();

  // Drop the peak by its fall rate
  peak -= PEAK_FALL_RATE;
}
