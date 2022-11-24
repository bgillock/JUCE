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
    double getMax();
    void setMax(double max);
    void init();
private:
    juce::SpinLock mutex;
    double maxAmp;
};