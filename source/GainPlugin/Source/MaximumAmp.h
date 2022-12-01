/*
  ==============================================================================

    MaximumAmp.h
    Created: 24 Nov 2022 9:51:30am
    Author:  bgill

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MaximumAmp
{
public:
    MaximumAmp();
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    void init();
    double getMax();
    int lastlight = 0;
    double peakhold = 0.0;
    int peakTimes = 0;
private:
    juce::SpinLock mutex;
    double maxAmp;
};