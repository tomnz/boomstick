#ifndef TIMESAMPLER_H
#define TIMESAMPLER_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif


class TimeSampler {
public:
    // Constructors
    TimeSampler(uint8_t secs);
    // Destructor
    //~TimeSampler();
    
    void Sample(int val);
    
    int Max();
    int Min();
    
private:
    unsigned long lastSampleMillis;
    uint8_t samples;
    uint8_t currSampleSlot;
    bool allSlotsUsed;
    int* minSamples;
    int* maxSamples;
    int currMax;
    int currMin;
};

// Helper functions

#endif // TIMESAMPLER_H
