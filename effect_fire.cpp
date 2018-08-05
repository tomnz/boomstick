#include "effect_fire.h"

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.


EffectFire::EffectFire() : Effect(FIRE_MIRROR) {
}

void EffectFire::loop(Lights *lights, float sanitizedLevel, double transformedLevel, double smoothedLevel, double historicLevel) {
  #ifdef FIRE_FRAME_DELAY_MS
  delay(FIRE_FRAME_DELAY_MS);
  #endif

  uint8_t cooling = map(sanitizedLevel*255, 0, 255, FIRE_COOLING_MIN, FIRE_COOLING_MAX);
  // Step 1.  Cool down every cell a little
  for (int i = 0; i < numPixels; i++) {
    heat[i] = qsub8(heat[i], random8(0, (cooling * 10 / numPixels) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = numPixels - 1; k >= 2; k--) {
    heat[k] = (heat[k-1] + heat[k-1] + heat[k-2]) / 3;
  }
  
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  uint8_t sparking = map(sanitizedLevel*255, 0, 255, FIRE_SPARKING_MIN, FIRE_SPARKING_MAX);
  uint8_t sparkHeat = map(sanitizedLevel*255, 0, 255, FIRE_SPARK_HEAT_MIN, FIRE_SPARK_HEAT_MAX);
  if (random8() < sparking) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(sparkHeat - 30, sparkHeat));
  }
  heat[0] = sparkHeat - 20;
  heat[1] = sparkHeat - 10;

  // Step 4.  Map from heat cells to LED colors
  for (int j = 0; j < numPixels; j++) {
    lights->setPixel(j, HeatColor(heat[j]));
  }
}
