#include <Adafruit_NeoPixel.h>
#include "Color.h"
#include "TimeSampler.h"

// LED hardware settings
#define LED_PIN     6     // NeoPixel LED strand is connected to this pin
#define N_PIXELS    16    // Number of pixels in strand

// Mic hardware settings
//#define MIC_PIN     A9    // Microphone is attached to this analog pin
#define MIC_PIN     1
#define DC_OFFSET   0     // DC offset in mic signal - if unusure, leave 0
#define NOISE       10    // Noise/hum/interference in mic signal
#define SAMPLES     60    // Length of buffer for dynamic level adjustment
//#define MIC_VOLT_REF    // Uncomment to have Arduino use the external analog reference voltage

// Mic animation settings
#define ENABLE_MIC
#define MIC_SAMPLES 60    // Seconds of min/max mic values to track
#define TOP         (N_PIXELS + 2) // Allow dot to go slightly off scale
#define PEAK_FALL   40    // Rate of peak falling dot

// Accelerometer animation settings
//#define ENABLE_ACCEL
#define ACCEL_SAMPLES 60  // Seconds of min/max values to track

// Globals
byte
    peak        = 0,            // Used for falling dot
    dotCount    = 0,            // Frame counter for delaying dot-falling speed
    volCount    = 0;            // Frame counter for storing past volume data
int
    lvl         = 10;           // Current "dampened" audio level
double
    volume      = 0.0;
Adafruit_NeoPixel
    strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#ifdef ENABLE_MIC
TimeSampler micSamples = TimeSampler(MIC_SAMPLES);
#endif

#ifdef ENABLE_ACCEL
TimeSampler accelSamples = TimeSampler(ACCEL_SAMPLES);
#endif

void setup() {
    // This is only needed on 5V Arduinos (Uno, Leonardo, etc.).
    // Connect 3.3V to mic AND TO AREF ON ARDUINO and enable this
    // line.    Audio samples are 'cleaner' at 3.3V.
#ifdef MIC_VOLT_REF
    analogReference(EXTERNAL);
#endif
    
    strip.begin();
}


void loop() {
#ifdef ENABLE_MIC
    DoMic();
#endif
#ifdef ENABLE_ACCEL
    DoAccel();
#endif
}

void DoMic() {
    uint8_t i;

    // Read raw mic level
    ReadMicLevel();
    micSamples.Sample(lvl);
    
    int minLvl = micSamples.Min(), maxLvl = micSamples.Max();
    
    // Calculate bar height based on dynamic min/max levels (fixed point):
    volume = ((double)lvl - (double)minLvl) / ((double)maxLvl - (double)minLvl);
    int height = (int)((double)TOP * volume);

    // Clip output
    if (height < 0)
        height = 0;
    else if (height > 1)
        height = 1;
    
    // Keep 'peak' dot at top
    if(height > peak) peak = height;

    // Color pixels based on rainbow gradient
    for (i=0; i<N_PIXELS; i++) {
        if (i >= height)
            strip.setPixelColor(i, 0, 0, 0);
        else
            strip.setPixelColor(i, Wheel(map(i, 0, strip.numPixels() - 1, 30, 150)));
        
    }

    // Draw peak dot    
    if (peak > 0 && peak <= N_PIXELS-1)
        strip.setPixelColor(peak, Wheel(map(peak, 0, strip.numPixels() - 1, 30, 150)));
    
    // Update strip
    strip.show();

    // Every few frames, make the peak pixel drop by 1:
    if(++dotCount >= PEAK_FALL) { //fall rate 
        if(peak > 0) peak--;
        dotCount = 0;
    }
}

void DoAccel() {
}

void ReadMicLevel() {
    // Raw reading from mic 
    int n = analogRead(MIC_PIN);
    // Center on zero
    n = abs(n - 512 - DC_OFFSET);
    // Remove noise/hum
    n = (n <= NOISE) ? 0 : (n - NOISE);

    // "Dampened" reading (else looks twitchy)
    lvl = ((lvl * 7) + n) >> 3;
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    if(WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}
