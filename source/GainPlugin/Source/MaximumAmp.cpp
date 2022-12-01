/*
  ==============================================================================

    MeterBuffer.cpp
    Created: 24 Nov 2022 9:51:30am
    Author:  bgill

  ==============================================================================
*/
#include <JuceHeader.h>
#include "MaximumAmp.h"

MaximumAmp::MaximumAmp() { maxAmp = -144.0; };
void MaximumAmp::capture(AudioBuffer<float> amps, int channel)
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        float db = Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
        if (db > maxAmp) maxAmp = db;
    }
}
void MaximumAmp::capture(AudioBuffer<double> amps, int channel)
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        float db = Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
        if (db > maxAmp) maxAmp = db;
    }
}
double MaximumAmp::getMax()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    return maxAmp;
}

void MaximumAmp::init()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        maxAmp = -144.0;
    }
}
