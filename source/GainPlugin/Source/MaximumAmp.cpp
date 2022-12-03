/*
  ==============================================================================

    MeterBuffer.cpp
    Created: 24 Nov 2022 9:51:30am
    Author:  bgill

  ==============================================================================
*/
#include <JuceHeader.h>
#include "MaximumAmp.h"

MaximumAmp::MaximumAmp(double min, double max, int nLevels, int peakHoldTimes) 
{ 
    _nLevels = nLevels;
    _minAmp = min;
    _maxAmp = max;
    _peakHoldTimes = peakHoldTimes;
    _levels = new float[_nLevels];
    peakAmp = -144.0; 
};
void MaximumAmp::setNLevels(int n)
{
    delete _levels;
    _nLevels = n;
    _levels = new float[_nLevels];
}
void MaximumAmp::capture(AudioBuffer<float> amps, int channel)
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        float db = Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
        if (db > peakAmp) peakAmp = db;
    }
}
void MaximumAmp::capture(AudioBuffer<double> amps, int channel)
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        float db = Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
        if (db > peakAmp) peakAmp = db;
    }
}
float* MaximumAmp::getLevels()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (++peakTimes > _peakHoldTimes)
    {
        peakTimes = 0;
        peakhold = -144.0;
    }

    if (peakAmp > peakhold)
    {
        peakhold = peakAmp;
        peakTimes = 0;
    }

    int l = (int)((float)(peakAmp - _minAmp) / (_maxAmp - _minAmp) * (float)_nLevels);
    int h = (int)((float)(peakhold - _minAmp) / (_maxAmp - _minAmp) * (float)_nLevels);
    for (int i = 0; i < _nLevels; i++) {
        _levels[i] = 0.0;
        if (i == h) _levels[i] = 1.0;
        if (i <= l) _levels[i] = 1.0;
    }
    return _levels;
}

void MaximumAmp::clear()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        peakAmp = -144.0;
    }
}
