/*
 * This script is an awful mish-mash of various code snippets found around
 * the internet. It works, but it's not pretty. Most of it is highly
 * customized from its derivative code.
 *
 * ffft is taken from an Adafruit sample here:
 * https://learn.adafruit.com/piccolo/
 * Needs the following modification to ffft.h to work with newer AVR:
 * https://github.com/adafruit/piccolo/pull/4
 *
 * Microphone input and some associated code is from the Adafruit sample here:
 * https://learn.adafruit.com/led-ampli-tie/the-code
 */

#include "config.h"
#include "FastLED.h"
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <ffft.h>
#include "lights.h"
#include "effect.h"

#ifdef EFFECT_BARS
#include "effect_bars.h"
#endif
#ifdef EFFECT_FIRE
#include "effect_fire.h"
#endif
#ifdef EFFECT_NOISE
#include "effect_noise.h"
#endif
#ifdef EFFECT_PULSE
#include "effect_pulse.h"
#endif
#ifdef EFFECT_SINELON
#include "effect_sinelon.h"
#endif

#ifdef ENABLE_SERIAL
#include <MemoryUsage.h>
#endif

// Microphone connects to Analog Pin 0.  Corresponding ADC channel number
// varies among boards...it's ADC0 on Uno and Mega, ADC7 on Leonardo.
// Other boards may require different settings; refer to datasheet.
#ifdef __AVR_ATmega32U4__
 #define ADC_CHANNEL 7
#else
 #define ADC_CHANNEL 0
#endif

int16_t       capture[FFT_N];    // Audio capture buffer
complex_t     bfly_buff[FFT_N];  // FFT "butterfly" buffer
uint16_t      spectrum[FFT_N/2]; // Spectrum output buffer
volatile uint16_t samplePos = 0;     // Buffer position counter

byte
  colCount = 0; // Frame counter for storing past column data
int
  col[HISTORIC_FRAMES],   // Column levels for the prior n frames
  minLvlAvg, // For dynamic adjustment of low & high ends of graph,
  maxLvlAvg, // pseudo rolling averages for the prior few frames.
  colDiv[8];    // Used when filtering FFT output to 8 columns

double smoothedLevel = 0;
double historicLevel = 0;
float sanitizedLevel = 0;


#ifdef BRIGHTNESS_PIN
volatile byte currentPin = BRIGHTNESS_PIN;
volatile bool brightnessUpdated = false;
volatile uint8_t brightness = 255;
#else
volatile byte currentPin = ADC_CHANNEL;
#endif


/*
These tables were arrived at through testing, modeling and trial and error,
exposing the unit to assorted music and sounds.  But there's no One Perfect
EQ Setting to Rule Them All, and the graph may respond better to some
inputs than others.  The software works at making the graph interesting,
but some columns will always be less lively than others, especially
comparing live speech against ambient music of varying genres.
*/
const uint8_t PROGMEM
  // This is low-level noise that's subtracted from each FFT output column:
  noise[64]={ 8,6,6,5,3,4,4,4,3,4,4,3,2,3,3,4,
              2,1,2,1,3,2,3,2,1,2,3,1,2,3,4,4,
              3,2,2,2,2,2,2,1,3,2,2,2,2,2,2,2,
              2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,4 },
  // These are scaling quotients for each FFT output column, sort of a
  // graphic EQ in reverse.  Most music is pretty heavy at the bass end.
  eq[64]={
    255, 175,218,225,220,198,147, 99, 68, 47, 33, 22, 14,  8,  4,  2,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
  // When filtering down to 8 columns, these tables contain indexes
  // and weightings of the FFT spectrum output values to use.  Not all
  // buckets are used -- the bottom-most and several at the top are
  // either noisy or out of range or generally not good for a graph.
  col0data[] = {  2,  1,  // # of spectrum bins to merge, index of first
    111,   8 },           // Weights for each bin
  col1data[] = {  4,  1,  // 4 bins, starting at index 1
     19, 186,  38,   2 }, // Weights for 4 bins.  Got it now?
  col2data[] = {  5,  2,
     11, 156, 118,  16,   1 },
  col3data[] = {  8,  3,
      5,  55, 165, 164,  71,  18,   4,   1 },
  col4data[] = { 11,  5,
      3,  24,  89, 169, 178, 118,  54,  20,   6,   2,   1 },
  col5data[] = { 17,  7,
      2,   9,  29,  70, 125, 172, 185, 162, 118, 74,
     41,  21,  10,   5,   2,   1,   1 },
  col6data[] = { 25, 11,
      1,   4,  11,  25,  49,  83, 121, 156, 180, 185,
    174, 149, 118,  87,  60,  40,  25,  16,  10,   6,
      4,   2,   1,   1,   1 },
  col7data[] = { 37, 16,
      1,   2,   5,  10,  18,  30,  46,  67,  92, 118,
    143, 164, 179, 185, 184, 174, 158, 139, 118,  97,
     77,  60,  45,  34,  25,  18,  13,   9,   7,   5,
      3,   2,   2,   1,   1,   1,   1 };
// And then this points to the start of the data for each of the columns:
const uint8_t * const colData[] PROGMEM = {
  col0data, col1data, col2data, col3data,
  col4data, col5data, col6data, col7data
};

Lights lights = Lights();

#ifdef EFFECT_BARS
EffectBars effectBars = EffectBars();
#endif
#ifdef EFFECT_PULSE
EffectPulse effectPulse = EffectPulse();
#endif
#ifdef EFFECT_NOISE
EffectNoise effectNoise = EffectNoise();
#endif
#ifdef EFFECT_SINELON
EffectSinelon effectSinelon = EffectSinelon();
#endif
#ifdef EFFECT_FIRE
EffectFire effectFire = EffectFire();
#endif

Effect* effects[N_EFFECTS] = {
#ifdef EFFECT_BARS
  &effectBars,
#endif
#ifdef EFFECT_PULSE
  &effectPulse,
#endif
#ifdef EFFECT_NOISE
  &effectNoise,
#endif
#ifdef EFFECT_SINELON
  &effectSinelon,
#endif
#ifdef EFFECT_FIRE
  &effectFire
#endif
};
uint8_t currentEffect = INITIAL_EFFECT;

#ifdef EFFECT_BUTTON_PIN
bool changedEffect = false;
#endif


void setup() {
  analogReference(EXTERNAL);

  // We support saving the last used effect between power cycles.
  // This uses the effect ID + 1 so we can discern from untouched memory.
  uint8_t effectMemory = EEPROM.read(0);
  if (effectMemory > 0 && effectMemory <= N_EFFECTS) {
    currentEffect = effectMemory - 1;
  }

#ifdef ENABLE_SERIAL
  const unsigned int BAUD_RATE = 9600;
  Serial.begin(BAUD_RATE);
#endif

  randomSeed(analogRead(0));

#ifdef EFFECT_BUTTON_PIN
  pinMode(EFFECT_BUTTON_PIN, INPUT);
#endif

  uint8_t i, j, nBins, *data;

  memset(col , 0, sizeof(col));

  minLvlAvg = 0;
  maxLvlAvg = 512;

  for (i = 0; i < 8; i++) {
    data         = (uint8_t *)pgm_read_word(&colData[i]);
    nBins        = pgm_read_byte(&data[0]) + 2;
    for (colDiv[i] = 0, j = 2; j < nBins; j++)
      colDiv[i] += pgm_read_byte(&data[j]);
  }

  // Init ADC free-run mode; f = ( 16MHz/prescaler ) / 13 cycles/conversion
  ADMUX  = currentPin; // Channel sel, right-adj, use AREF pin
  ADCSRA = _BV(ADEN)  | // ADC enable
           _BV(ADSC)  | // ADC start
           _BV(ADATE) | // Auto trigger
           _BV(ADIE)  | // Interrupt enable
           _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 128:1 / 13 = 9615 Hz
  ADCSRB = 0x40;                // Free run mode, no high MUX bit
  DIDR0  = 1 << ADC_CHANNEL; // Turn off digital input for ADC pin
#ifdef BRIGHTNESS_PIN
  DIDR0 |= 1 << BRIGHTNESS_PIN;
#endif
  // TODO: Anecdotally, does disabling this worsen FFT quality?
  // With the timer disabled, millis() no longer updates, but supposedly this
  // increases jitter/noise for the ADC
  //TIMSK0 = 0;                // Timer0 off

  sei(); // Enable interrupts
}


void loop() {
#ifdef ENABLE_SERIAL
  EVERY_N_SECONDS(5) {
    Serial.print(F("FPS: "));
    Serial.println(FastLED.getFPS());
    FREERAM_PRINT
    Serial.println();
  }
#endif

  uint8_t  i, x, L, *data, nBins, binNum, c;
  uint16_t minLvl, maxLvl, currLevel;
  int minLevelCurrent, maxLevelCurrent, y, sum;
  double transformedLevel;

#ifdef EFFECT_BUTTON_PIN
  // Change the effect if necessary
  if (digitalRead(EFFECT_BUTTON_PIN) == HIGH) {
    if (!changedEffect) {
      changedEffect = true;
      currentEffect = (currentEffect + 1) % N_EFFECTS;
      EEPROM.write(0, currentEffect + 1);
    }
  } else if (changedEffect) {
    changedEffect = false;
  }
#endif

  // Wait for analog sampling to finish
  while (ADCSRA & _BV(ADIE));
  samplePos = 0;

#ifdef BRIGHTNESS_PIN
  // Set brightness
  if (brightnessUpdated) {
    lights.setBrightness(brightness);

    brightnessUpdated = false;
    // Don't bother doing anything else in this loop
    return;
  }
#endif

  // Re-enable the ADC interrupt
  ADCSRA |= _BV(ADIE);

  fft_input(capture, bfly_buff);   // Samples -> complex #s
  fft_execute(bfly_buff);          // Process complex data
  fft_output(bfly_buff, spectrum); // Complex -> spectrum

  // Remove noise and apply EQ levels
  for (x = 0; x < FFT_N / 2; x++) {
    L = pgm_read_byte(&noise[x]);
    spectrum[x] = (spectrum[x] <= L) ? 0 :
      (((spectrum[x] - L) * (256L - pgm_read_byte(&eq[x]))) >> 8);
  }

  // Downsample spectrum output to 8 columns:
  x = FFT_SLOT;
  data   = (uint8_t *)pgm_read_word(&colData[x]);
  nBins  = pgm_read_byte(&data[0]) + 2;
  binNum = pgm_read_byte(&data[1]);

  for (sum = 0, i = 2; i < nBins; i++) {
    sum += spectrum[binNum++] * pgm_read_byte(&data[i]); // Weighted
  }
  col[colCount] = sum / colDiv[x];                    // Average

#ifdef FFT_SLOT2
  x = FFT_SLOT2;
  data   = (uint8_t *)pgm_read_word(&colData[x]);
  nBins  = pgm_read_byte(&data[0]) + 2;
  binNum = pgm_read_byte(&data[1]);

  for (sum = 0, i = 2; i < nBins; i++) {
    sum += spectrum[binNum++] * pgm_read_byte(&data[i]); // Weighted
  }

  // Possible combinations with previous reading
  //col[colCount] += sum / colDiv[x];
  col[colCount] = max(col[colCount], sum / colDiv[x]);
#endif

  currLevel = (col[colCount] < LEVEL_CUTOFF) ? 0 : col[colCount];
  minLvl = maxLvl = col[0];
  for (i = 1; i < HISTORIC_FRAMES; i++) { // Get range of prior 10 frames
    if (col[i] < minLvl)      minLvl = col[i];
    else if (col[i] > maxLvl) maxLvl = col[i];
  }
  // minLvl and maxLvl indicate the extents of the FFT output, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  minLvlAvg = (minLvlAvg * 7 + minLvl) >> 3; // Dampen min/max levels
  maxLvlAvg = (maxLvlAvg * 7 + maxLvl) >> 3; // (fake rolling average)
  if (maxLvl > maxLvlAvg) {
    maxLvlAvg = maxLvl;
  }

  // Second fixed-point scale based on dynamic min/max levels:
  smoothedLevel = (smoothedLevel * SMOOTH_FACTOR + (double)currLevel) / (SMOOTH_FACTOR + 1.0);
  double historicSmoothFactor = HISTORIC_SMOOTH_FACTOR_DOWN;
  if (smoothedLevel > historicLevel) {
    historicSmoothFactor = HISTORIC_SMOOTH_FACTOR_UP;
  }
  historicLevel = (historicLevel * historicSmoothFactor + (double)smoothedLevel) / (historicSmoothFactor + 1.0);

  minLevelCurrent = max(historicLevel * 1.2, minLvlAvg);
  maxLevelCurrent = max(maxLvlAvg * 1.2, minLevelCurrent + max(MIN_BAR_SIZE, historicLevel * 0.7));

  transformedLevel = ((double)(BASE_LEVEL_SCALE * smoothedLevel - (double)minLevelCurrent) /
    ((double)maxLevelCurrent - (double)minLevelCurrent));

  float newSanitizedLevel = (transformedLevel - SANITIZED_LEVEL_MIN) * SANITIZED_LEVEL_SCALE;
  if (newSanitizedLevel > sanitizedLevel) {
    sanitizedLevel = min(newSanitizedLevel, 1.0);
  } else {
    sanitizedLevel -= SANITIZED_LEVEL_DECAY;
  }
  sanitizedLevel = max(0.0, sanitizedLevel);

  // Call out to given effect
  effects[currentEffect]->loop(&lights, sanitizedLevel, transformedLevel, smoothedLevel, historicLevel);
  lights.show(effects[currentEffect]->mirror);

  // if (smoothedLevel < historicLevel * 0.4) {
  //   smoothedLevel = 0;
  // }

  if (++colCount >= HISTORIC_FRAMES) colCount = 0;
}

ISR(ADC_vect) { // Audio-sampling interrupt
  int sample = ADCL; // 0-1023
  sample += ADCH << 8;

#ifdef BRIGHTNESS_PIN
  if (currentPin == BRIGHTNESS_PIN) {
    if (++samplePos >= 2) {
      // Sample brightness
      brightness = (uint8_t)map(sample, 0, 1024, 5, 255);
      brightnessUpdated = true;

      // Disable interrupt
      ADCSRA &= ~_BV(ADIE);
      // Switch back to audio for next time
      currentPin = ADC_CHANNEL;
      ADMUX = currentPin;
    }
  } else {
#endif
    capture[samplePos] =
      ((sample > (512-NOISE_THRESHOLD)) &&
       (sample < (512+NOISE_THRESHOLD))) ? 0 :
      sample - 512; // Sign-convert for FFT; -512 to +511


    if (++samplePos >= FFT_N) {
      ADCSRA &= ~_BV(ADIE); // Buffer full, interrupt off
#ifdef BRIGHTNESS_PIN
      // Get a brightness reading next time
      currentPin = BRIGHTNESS_PIN;
      ADMUX = currentPin;
#endif
    }
#ifdef BRIGHTNESS_PIN
  }
#endif
}
