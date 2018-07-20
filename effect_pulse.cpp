#include "effect_pulse.h"

EffectPulse::EffectPulse():
  beatOn(false), lowFrames(0)
{
  chooseNewColor();

  for (int i = 0; i < N_PIXELS; i++) {
    brightness[i] = 1.0;
  }
}

void EffectPulse::chooseNewColor() {
#ifdef PULSE_RANDOM
  currentColorPos = random(0, 255);
#else
  currentColorPos = (uint8_t)(((uint16_t)currentColorPos + PULSE_COLOR_INCREMENT) % 255);
#endif

  currentColor = Wheel(currentColorPos);
}

void EffectPulse::loop(Lights *lights, double transformedLevel, double smoothedLevel, double historicLevel) {
  // Detect beat
  bool newBeat = false;
  uint8_t i;

  if (beatOn) {
    if (lowFrames >= BEAT_LOW_FRAMES) {
      // Disable current beat
      beatOn = false;
      lowFrames = 0;
    } else if (transformedLevel < BEAT_LOW) {
      ++lowFrames;
    }
  } else {
    // Check for a new beat
    if (transformedLevel > BEAT_HIGH) {
      beatOn = true;
      newBeat = true;
    }
  }

  if (newBeat) {
    chooseNewColor();

    // Randomly boost the LEDs on a beat
    float randVal;
    for (i = 0; i < N_PIXELS; i++) {
      randVal = ((float)(random(0, 100)) * PULSE_BRIGHTNESS_BEAT) / 100.0;
      brightness[i] = randVal * randVal;
    }
  }

  // Pull the brightnesses towards 1.0
  for (i = 0; i < N_PIXELS; i++) {
    brightness[i] = brightness[i] * PULSE_FADE_FACTOR / (PULSE_FADE_FACTOR + 1.0);
  }

  // Clamp brightness
  Color color = Color();
  float currBrightness;
  for (i = 0; i < N_PIXELS; i++) {
    currBrightness = min(1.0, max(PULSE_MIN_BRIGHTNESS, min(0.3, transformedLevel / 4.0 + PULSE_BRIGHTNESS_BOOST)) + brightness[i]);

    color.r = (int)((float)currentColor.r * currBrightness);
    color.g = (int)((float)currentColor.g * currBrightness);
    color.b = (int)((float)currentColor.b * currBrightness);

    lights->setPixel(i, color);
  }

  lights->show();
}
