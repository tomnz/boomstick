#include <Adafruit_NeoPixel.h>
#include "color.h"

// LED hardware settings
#define LED_PIN     6     // NeoPixel LED strand is connected to this pin
#define N_PIXELS    16    // Number of pixels in strand

// Mic hardware settings
#define MIC_PIN     A9    // Microphone is attached to this analog pin
#define DC_OFFSET   0     // DC offset in mic signal - if unusure, leave 0
#define NOISE       10    // Noise/hum/interference in mic signal
#define SAMPLES     60    // Length of buffer for dynamic level adjustment
//#define MIC_VOLT_REF    // Uncomment to have Arduino use the external analog reference volatage

// Mic animation settings
#define ENABLE_MIC
#define TOP         (N_PIXELS + 2) // Allow dot to go slightly off scale
#define PEAK_FALL   40    // Rate of peak falling dot

// Accelerometer animation settings
//#define ENABLE_ACCEL

byte
    peak        = 0,            // Used for falling dot
    dotCount    = 0,            // Frame counter for delaying dot-falling speed
    volCount    = 0;            // Frame counter for storing past volume data
int
    vol[SAMPLES],             // Collection of prior volume samples
    lvl         = 10,            // Current "dampened" audio level
    minLvlAvg   = 0,            // For dynamic adjustment of graph low & high
    maxLvlAvg   = 512;
Adafruit_NeoPixel
    strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);


void setup() {
#ifdef ENABLE_MIC

    // This is only needed on 5V Arduinos (Uno, Leonardo, etc.).
    // Connect 3.3V to mic AND TO AREF ON ARDUINO and enable this
    // line.    Audio samples are 'cleaner' at 3.3V.
#ifdef MIC_VOLT_REF
    analogReference(EXTERNAL);
#endif

    // Reset historical volume samples
    memset(vol, 0, sizeof(vol));
    
#endif    
    
    strip.begin();
}


void loop() {
#ifdef ENABLE_MIC
    DoMic();
#endif
#ifdef ENABLE_ACCEL
#endif
}

void DoMic() {
    uint8_t i;

    // Read raw mic level (also updates lvl)
    int n = ReadMicLevel();
    
    // Calculate bar height based on dynamic min/max levels (fixed point):
    int height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

    // Clip output
    if (height < 0L)
        height = 0;
    else if (height > TOP)
        height = TOP;
    
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

    // Save sample for dynamic leveling
    vol[volCount] = n;
    // Advance/rollover sample counter
    if(++volCount >= SAMPLES) volCount = 0;

    // Get volume range of prior frames
    uint16_t minLvl, maxLvl;
    minLvl = maxLvl = vol[0];
    for(i=1; i<SAMPLES; i++) {
        if (vol[i] < minLvl)
            minLvl = vol[i];
        else if (vol[i] > maxLvl)
            maxLvl = vol[i];
    }
    
    // minLvl and maxLvl indicate the volume range over prior frames, used
    // for vertically scaling the output graph (so it looks interesting
    // regardless of volume level).    If they're too close together though
    // (e.g. at very low volume levels) the graph becomes super coarse
    // and 'jumpy'...so keep some minimum distance between them (this
    // also lets the graph go to zero when no sound is playing):
    if((maxLvl - minLvl) < TOP)
        maxLvl = minLvl + TOP;
    
    // Dampen min/max levels
    // (fake rolling average)
    minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6;
    maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6;
}

int ReadMicLevel() {
    // Raw reading from mic 
    int n = analogRead(MIC_PIN);
    // Center on zero
    n = abs(n - 512 - DC_OFFSET);
    // Remove noise/hum
    n = (n <= NOISE) ? 0 : (n - NOISE);

    // "Dampened" reading (else looks twitchy)
    lvl = ((lvl * 7) + n) >> 3;
    
    return n;
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
