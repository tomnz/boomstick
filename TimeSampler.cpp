#include "TimeSampler.h"

#define MILLIS_PER_SECOND 1000

TimeSampler::TimeSampler(uint8_t secs) : currSampleSlot(0), allSlotsUsed(false), samples(secs) {
    lastSampleMillis = millis();
    int tempMinSamples[samples];
    minSamples = tempMinSamples;
    int tempMaxSamples[samples];
    maxSamples = tempMaxSamples;
}

void TimeSampler::Sample(int val) {
    // See if we should add another sample
    unsigned long currMillis = millis();
    if (currMillis - lastSampleMillis >= MILLIS_PER_SECOND) {
        currSampleSlot++;
        if (currSampleSlot >= samples) {
            currSampleSlot = 0;
            allSlotsUsed = true;
        }
        minSamples[currSampleSlot] = currMin;
        maxSamples[currSampleSlot] = currMax;
        currMax = val;
        currMin = val;
    }
    
    if (val > currMax) {
        currMax = val;
    }
    if (val < currMin) {
        currMin = val;
    }
}

int TimeSampler::Max() {
    int slots = allSlotsUsed ? samples : currSampleSlot;
    
    int maxVal = currMax;
    for (int i = 0; i < slots; i++) {
        if (maxSamples[i] > maxVal) {
            maxVal = maxSamples[i];
        }
    }
    
    return maxVal;
}

int TimeSampler::Min() {
    int slots = allSlotsUsed ? samples : currSampleSlot;
    
    int minVal = currMin;
    for (int i = 0; i < slots; i++) {
        if (minSamples[i] > minVal) {
            minVal = minSamples[i];
        }
    }
    
    return minVal;
}
