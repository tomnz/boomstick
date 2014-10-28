// IMPORTANT: FFT_N should be #defined as 128 in ffft.h.  This is different
// than Spectro, which requires FFT_N be 64 in that file when compiling.

#include <avr/pgmspace.h>
#include <ffft.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// LED hardware settings
#define LED_PIN     6     // NeoPixel LED strand is connected to this pin
#define N_PIXELS    60    // Number of pixels in strand
#define TOP         (N_PIXELS + 2) // Allow dot to go slightly off scale

// Animation settings
#define FFT_SLOT    1     // Which FFT index (0-7) to pull level data from
#define HISTORIC_SMOOTH_FACTOR 500.0
#define HISTORIC_SCALE 1.5
#define SMOOTH_FACTOR 5.0
#define MIN_COL     50
#define MAX_COL     255
#define COL_RANGE   (MAX_COL - MIN_COL)
#define COL_VAR     40

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
  peak[8],      // Peak level of each column; used for falling dots
  dotCount = 0, // Frame counter for delaying dot-falling speed
  colCount = 0; // Frame counter for storing past column data
int
  col[8][10],   // Column levels for the prior 10 frames
  minLvlAvg[8], // For dynamic adjustment of low & high ends of graph,
  maxLvlAvg[8], // pseudo rolling averages for the prior few frames.
  colDiv[8];    // Used when filtering FFT output to 8 columns

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
PROGMEM uint8_t
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
      3,   2,   2,   1,   1,   1,   1 },
  // And then this points to the start of the data for each of the columns:
  *colData[] = {
    col0data, col1data, col2data, col3data,
    col4data, col5data, col6data, col7data };

Adafruit_NeoPixel
	strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  
  const unsigned int BAUD_RATE = 9600;
  Serial.begin(BAUD_RATE);

  uint8_t i, j, nBins, binNum, *data;

  memset(peak, 0, sizeof(peak));
  memset(col , 0, sizeof(col));

  for(i=0; i<8; i++) {
    minLvlAvg[i] = 0;
    maxLvlAvg[i] = 512;
    data         = (uint8_t *)pgm_read_word(&colData[i]);
    nBins        = pgm_read_byte(&data[0]) + 2;
    binNum       = pgm_read_byte(&data[1]);
    for(colDiv[i]=0, j=2; j<nBins; j++)
      colDiv[i] += pgm_read_byte(&data[j]);
  }

  strip.begin();

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
  uint8_t  i, x, L, *data, nBins, binNum, weighting, c;
  uint16_t minLvl, maxLvl, currLevel;
  int      level, y, sum;

  while(ADCSRA & _BV(ADIE)); // Wait for audio sampling to finish

  fft_input(capture, bfly_buff);   // Samples -> complex #s
  samplePos = 0;                   // Reset sample counter
  ADCSRA |= _BV(ADIE);             // Resume sampling interrupt
  fft_execute(bfly_buff);          // Process complex data
  fft_output(bfly_buff, spectrum); // Complex -> spectrum

  // Remove noise and apply EQ levels
  for(x=0; x<FFT_N/2; x++) {
    L = pgm_read_byte(&noise[x]);
    spectrum[x] = (spectrum[x] <= L) ? 0 :
      (((spectrum[x] - L) * (256L - pgm_read_byte(&eq[x]))) >> 8);
  }

  // Downsample spectrum output to 8 columns:
  x = FFT_SLOT;
  data   = (uint8_t *)pgm_read_word(&colData[x]);
  nBins  = pgm_read_byte(&data[0]) + 2;
  binNum = pgm_read_byte(&data[1]);
  for(sum=0, i=2; i<nBins; i++)
    sum += spectrum[binNum++] * pgm_read_byte(&data[i]); // Weighted
  currLevel = col[x][colCount] = sum / colDiv[x];                    // Average
  minLvl = maxLvl = col[x][0];
  for(i=1; i<10; i++) { // Get range of prior 10 frames
    if(col[x][i] < minLvl)      minLvl = col[x][i];
    else if(col[x][i] > maxLvl) maxLvl = col[x][i];
  }
  // minLvl and maxLvl indicate the extents of the FFT output, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if((maxLvl - minLvl) < 8) maxLvl = minLvl + 8;
  minLvlAvg[x] = (minLvlAvg[x] * 7 + minLvl) >> 3; // Dampen min/max levels
  maxLvlAvg[x] = (maxLvlAvg[x] * 7 + maxLvl) >> 3; // (fake rolling average)

  // Second fixed-point scale based on dynamic min/max levels:
  lastLevel = (lastLevel * SMOOTH_FACTOR + (double)currLevel) / (SMOOTH_FACTOR + 1.0);
  
  level = (int)((TOP * lastLevel - TOP * (double)(minLvlAvg[x])) /
    ((double)(maxLvlAvg[x]) - (double)(minLvlAvg[x])));

  // Clip output and convert to byte:
  if(level < 0L)      c = 0;
  else if(level > TOP) c = TOP; // Allow dot to go a couple pixels off top
  else                c = (uint8_t)level;

  if(c > peak[x]) peak[x] = c; // Keep dot on top

  if (!haveHistoricVolume) {
    haveHistoricVolume = true;
    historicVolume = currLevel;
  }
  
  historicVolume = (historicVolume * HISTORIC_SMOOTH_FACTOR + (double)maxLvlAvg[x]) / (HISTORIC_SMOOTH_FACTOR + 1.0);
  int volumeEffect = (((COL_RANGE - COL_VAR)/2) * (double)maxLvlAvg[x] * HISTORIC_SCALE / historicVolume) + (COL_VAR/2);
  
  if (volumeEffect > (COL_RANGE - COL_VAR/2))
    volumeEffect = (COL_RANGE - COL_VAR/2);
  if (volumeEffect < (COL_VAR / 2))
    volumeEffect = (COL_VAR / 2);

  // Color pixels based on rainbow gradient
  for (i=0; i<N_PIXELS; i++) {
    if (i >= level)
      strip.setPixelColor(i, 0, 0, 0);
    else
      strip.setPixelColor(i, Wheel(volumeEffect + map(i, 0, strip.numPixels() - 1, 0, COL_VAR) + MIN_COL));
  }

  // Draw peak dot    
  if (peak[x] > 0 && peak[x] <= N_PIXELS-1)
    strip.setPixelColor(peak[x], Wheel(volumeEffect + map(i, 0, strip.numPixels() - 1, 0, COL_VAR) + MIN_COL));

  strip.show();

  // Every third frame, make the peak pixels drop by 1:
  if(++dotCount >= 3) {
    dotCount = 0;
    for(x=0; x<8; x++) {
      if(peak[x] > 0) peak[x]--;
    }
  }

  if(++colCount >= 10) colCount = 0;
}

ISR(ADC_vect) { // Audio-sampling interrupt
  static const int16_t noiseThreshold = 4;
  int16_t              sample         = ADC; // 0-1023

  capture[samplePos] =
    ((sample > (512-noiseThreshold)) &&
     (sample < (512+noiseThreshold))) ? 0 :
    sample - 512; // Sign-convert for FFT; -512 to +511
    
  if(++samplePos >= FFT_N) ADCSRA &= ~_BV(ADIE); // Buffer full, interrupt off
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
//  WheelPos = 255 - WheelPos;
  
	if(WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	} else if(WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	} else {
		WheelPos -= 170;
		return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
}
