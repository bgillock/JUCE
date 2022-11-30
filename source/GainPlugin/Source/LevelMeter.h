/*
  ==============================================================================

    LevelMeter.h
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


//==============================================================================
/*
*/
class LevelMeter  : public juce::Component,
    public Timer
{
public:
    LevelMeter();
    void paint (juce::Graphics&) override;
    void LevelMeter::timerCallback() override;
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    void init();
    double getMax();
private:
    juce::SpinLock mutex;
    double maxAmp;
private:
 
    int lastlight = 0;
    double peakhold = 0.0;
    const int peakholdTimes = 10; // number of times to leave peak 
    int peakTimes = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
