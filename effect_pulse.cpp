#include "effect_pulse.h"

EffectPulse::EffectPulse():
  beatOn(false), lowFrames(0)
{
  chooseNewColor();
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
  }

  // Clamp brightness
  double brightness = max(PULSE_MIN_BRIGHTNESS, min(1.0, transformedLevel + PULSE_BRIGHTNESS_BOOST));

  Color color = Color(
    (int)((double)currentColor.r * brightness),
    (int)((double)currentColor.g * brightness),
    (int)((double)currentColor.b * brightness)
  );

  for (int i = 0; i < N_PIXELS; i++) {
    lights->setPixel(i, color);
  }

  lights->show();
}
