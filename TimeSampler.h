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
    bool allSlotsUsed;
    uint8_t
        samples,
        currSampleSlot;
    int
        *minSamples,
        *maxSamples;
    int
        currMax,
        currMin;
};

// Helper functions

#endif // TIMESAMPLER_H
