/*
  ==============================================================================

    LevelMeter.h
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MaximumAmp.h"

//==============================================================================
/*
*/
class LevelMeter  : public juce::Component,
    public Timer
{
public:
    LevelMeter(MaximumAmp &);

    void paint (juce::Graphics&) override;
    void LevelMeter::timerCallback() override;
private:
    MaximumAmp* maxAmp;
    int lastlight = 0;
    double peakhold = 0.0;
    const int peakholdTimes = 10; // number of times to leave peak 
    int peakTimes = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
