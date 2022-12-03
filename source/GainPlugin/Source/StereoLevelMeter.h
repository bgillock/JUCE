/*
  ==============================================================================

    StereoLevelMeter.h
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MaximumAmp.h"

//==============================================================================
class LevelMeter : public juce::Component
{
public:
    LevelMeter();
    void paint(juce::Graphics&) override;
    void resized() override;
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    void init();
private:
    MaximumAmp maxAmp;
    const int peakholdTimes = 10; // number of times to leave peak 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter);
};

class StereoLevelMeter : public juce::Component,
    public Timer
{
public:
    StereoLevelMeter();
    void resized() override;
    void capture(AudioBuffer<float> amps);
    void capture(AudioBuffer<double> amps);
    void init();
    void StereoLevelMeter::timerCallback() override;
private:

    LevelMeter leftLevelMeter;
    LevelMeter rightLevelMeter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoLevelMeter);
};

