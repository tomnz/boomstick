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
#include <avr/pgmspace.h>
#include <ffft.h>
#include <Adafruit_NeoPixel.h>
#include "boomstick.h"

//#define ENABLE_SERIAL

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
volatile byte samplePos = 0;     // Buffer position counter

byte
  dotCount = 0, // Frame counter for delaying dot-falling speed
  colCount = 0; // Frame counter for storing past column data
int
  col[HISTORIC_FRAMES],   // Column levels for the prior n frames
  minLvlAvg, // For dynamic adjustment of low & high ends of graph,
  maxLvlAvg, // pseudo rolling averages for the prior few frames.
  colDiv[8],    // Used when filtering FFT output to 8 columns
  background = 0;
float
  peak,         // Peak level; used for falling dots
  backgroundLevel = 0.0; // Brightness of background

double lastLevel = 0;

double historicVolume;
boolean haveHistoricVolume = false;

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
    col4data, col5data, col6data, col7data };

// For mirrored strings, we need to double the number of pixels
#ifdef MIRROR_DUPE
Adafruit_NeoPixel
	strip = Adafruit_NeoPixel(N_PIXELS * 2, LED_PIN, NEO_GRB + NEO_KHZ800);
#else
Adafruit_NeoPixel
	strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

#ifdef LED_PIN2
#ifdef MIRROR_DUPE
Adafruit_NeoPixel
	strip2 = Adafruit_NeoPixel(N_PIXELS * 2, LED_PIN2, NEO_GRB + NEO_KHZ800);
#else
Adafruit_NeoPixel
	strip2 = Adafruit_NeoPixel(N_PIXELS, LED_PIN2, NEO_GRB + NEO_KHZ800);
#endif
#endif

void setup() {
  analogReference(EXTERNAL);

#ifdef ENABLE_SERIAL
  const unsigned int BAUD_RATE = 9600;
  Serial.begin(BAUD_RATE);
#endif

  uint8_t i, j, nBins, *data;

  memset(col , 0, sizeof(col));

  minLvlAvg = 0;
  maxLvlAvg = 512;

  for(i=0; i<8; i++) {
    data         = (uint8_t *)pgm_read_word(&colData[i]);
    nBins        = pgm_read_byte(&data[0]) + 2;
    for(colDiv[i]=0, j=2; j<nBins; j++)
      colDiv[i] += pgm_read_byte(&data[j]);
  }

  strip.begin();

#ifdef LED_PIN2
  strip2.begin();
#endif

  // Init ADC free-run mode; f = ( 16MHz/prescaler ) / 13 cycles/conversion
  ADMUX  = ADC_CHANNEL; // Channel sel, right-adj, use AREF pin
  ADCSRA = _BV(ADEN)  | // ADC enable
           _BV(ADSC)  | // ADC start
           _BV(ADATE) | // Auto trigger
           _BV(ADIE)  | // Interrupt enable
           _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 128:1 / 13 = 9615 Hz
  ADCSRB = 0;                // Free run mode, no high MUX bit
  DIDR0  = 1 << ADC_CHANNEL; // Turn off digital input for ADC pin
  TIMSK0 = 0;                // Timer0 off

  sei(); // Enable interrupts
}


void loop() {
  uint8_t  i, x, L, *data, nBins, binNum, c;
  uint16_t minLvl, maxLvl, currLevel;
  Color color;
  Color backgroundColor;
  int      level, y, sum;

  while(ADCSRA & _BV(ADIE)); // Wait for audio sampling to finish

  ADCSRA |= _BV(ADIE);

  fft_input(capture, bfly_buff);   // Samples -> complex #s
  samplePos = 0;                   // Reset sample counter
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
  if ((maxLvl - minLvl) < 8) {
    maxLvl = minLvl + MIN_BAR_SIZE;
  }
  minLvlAvg = (minLvlAvg * 7 + minLvl) >> 3; // Dampen min/max levels
  maxLvlAvg = (maxLvlAvg * 7 + maxLvl) >> 3; // (fake rolling average)

  // Second fixed-point scale based on dynamic min/max levels:
  lastLevel = (lastLevel * SMOOTH_FACTOR + (double)currLevel) / (SMOOTH_FACTOR + 1.0);

  level = (int)((TOP * BAR_SCALE * lastLevel - TOP * (double)minLvlAvg) /
    ((double)maxLvlAvg - (double)minLvlAvg));

  // Clip output and convert to byte:
  if (level < 0L)       c = 0;
  else if (level > TOP) c = TOP; // Allow dot to go a couple pixels off top
  else                  c = (uint8_t)level;

  if (level <= BACKGROUND_CUTOFF && backgroundLevel < BACKGROUND_MAX) {
    backgroundLevel += BACKGROUND_INCREASE;
  }
  else if (level > BACKGROUND_CUTOFF && backgroundLevel > 0) {
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

  if (c > peak) {
    peak = c; // Keep dot on top
  }

  if (!haveHistoricVolume) {
    haveHistoricVolume = true;
    historicVolume = currLevel;
  }

  historicVolume = (historicVolume * HISTORIC_SMOOTH_FACTOR + (double)lastLevel) / (HISTORIC_SMOOTH_FACTOR + 1.0);
  int volumeEffect = (((COL_RANGE - COL_VAR)/2) * (double)lastLevel * HISTORIC_SCALE / historicVolume) + (COL_VAR/2);

  if (lastLevel < historicVolume / 2) {
    lastLevel = 0;
    level = 0;
  }

  if (volumeEffect > (COL_RANGE - COL_VAR/2)) {
    volumeEffect = (COL_RANGE - COL_VAR/2);
  }
  if (volumeEffect < (COL_VAR / 2)) {
    volumeEffect = (COL_VAR / 2);
  }

  // Color pixels based on rainbow gradient
  strip.clear();
  strip.setBrightness(255);
#ifdef LED_PIN2
  strip2.clear();
  strip2.setBrightness(255);
#endif
  for (i = 0; i < N_PIXELS; i++) {
    if (i >= level) {
      setPixel(i, backgroundColor);
    }
    else {
      color = Wheel(volumeEffect + map(i, 0, strip.numPixels() - 1, 0, COL_VAR) + MIN_COL);
      setPixel(i, color);
    }
  }

  // Draw peak dot
  if (peak > 2) {
    color = Wheel(volumeEffect + map(peak, 0, strip.numPixels() - 1, 0, COL_VAR) + MIN_COL);
    for (i = max((int)peak - 2, level); i < min((int)peak + 2, N_PIXELS - 1); i++) {
      float intensity = 1.0 - min(abs((float)i - peak) / 1.5, 1.0);
      Color peakColor = Color((int)((float)color.r * intensity), (int)((float)color.g * intensity), (int)((float)color.b * intensity));
      setPixel(i, peakColor);
    }
  }


  strip.show();
#ifdef LED_PIN2
  strip2.show();
#endif

  // Drop the peak by its fall rate
  peak -= PEAK_FALL_RATE;

  if (++colCount >= HISTORIC_FRAMES) colCount = 0;
}

ISR(ADC_vect) { // Audio-sampling interrupt
  int16_t sample = ADC; // 0-1023

  capture[samplePos] =
    ((sample > (512-NOISE_THRESHOLD)) &&
     (sample < (512+NOISE_THRESHOLD))) ? 0 :
    sample - 512; // Sign-convert for FFT; -512 to +511

  if(++samplePos >= FFT_N) ADCSRA &= ~_BV(ADIE); // Buffer full, interrupt off
}

void setPixel(int idx, Color color) {
#ifdef FLIP
    idx = N_PIXELS - idx - 1;
#endif

  strip.setPixelColor(idx, color.r, color.g, color.b);
#ifdef LED_PIN2
  strip2.setPixelColor(idx, color.r, color.g, color.b);
#endif
#ifdef MIRROR_DUPE
  strip.setPixelColor(N_PIXELS * 2 - idx - 1, color.r, color.g, color.b);
#ifdef LED_PIN2
  strip2.setPixelColor(N_PIXELS * 2 - idx - 1, color.r, color.g, color.b);
#endif
#endif
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
Color Wheel(byte wheelPos) {
//  WheelPos = 255 - WheelPos;

	if(wheelPos < 85) {
		return Color(255 - wheelPos * 3, 0, wheelPos * 3);
	} else if(wheelPos < 170) {
		wheelPos -= 85;
		return Color(0, wheelPos * 3, 255 - wheelPos * 3);
	} else {
		wheelPos -= 170;
		return Color(wheelPos * 3, 255 - wheelPos * 3, 0);
	}
}
