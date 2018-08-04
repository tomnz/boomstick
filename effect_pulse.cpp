#include "effect_pulse.h"

EffectPulse::EffectPulse() : Effect(PULSE_MIRROR), beatOn(false), lowFrames(0) {
  chooseNewColor();

  for (int i = 0; i < numPixels; i++) {
    brightness[i] = 1.0;
  }
}

void EffectPulse::chooseNewColor() {
#ifdef PULSE_RANDOM
  currentColorPos = random(0, 255);
#else
  currentColorPos = (uint8_t)(((uint16_t)currentColorPos + PULSE_COLOR_INCREMENT) % 255);
#endif

  currentColor = CHSV(currentColorPos, 255, 255);
}

void EffectPulse::loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel) {
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

    // Randomly boost the LEDs on a beat
    float randVal;
    for (uint8_t i = 0; i < numPixels; i++) {
      randVal = ((float)(random8(255)) * PULSE_BRIGHTNESS_BEAT) / 255.0;
      brightness[i] = randVal * randVal;
    }
  }

  // Pull the brightnesses towards 1.0
  for (uint8_t i = 0; i < numPixels; i++) {
    brightness[i] = brightness[i] * PULSE_FADE_FACTOR / (PULSE_FADE_FACTOR + 1.0);
  }

  // Clamp brightness
  for (uint8_t i = 0; i < numPixels; i++) {
    uint8_t pixelBrightness = (uint8_t)(255.0 * min(1.0, max(PULSE_MIN_BRIGHTNESS, min(0.3, transformedLevel / 4.0 + PULSE_BRIGHTNESS_BOOST)) + brightness[i]));
    CRGB pixelColor = CRGB(currentColor);
    pixelColor.nscale8_video(pixelBrightness);

    lights->setPixel(i, pixelColor);
  }
}
