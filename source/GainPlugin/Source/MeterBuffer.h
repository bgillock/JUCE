/*
  ==============================================================================

    MaximumAmp.h
    Created: 24 Nov 2022 9:51:30am
    Author:  bgill

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
class MeterBuffer
{
public:
    MeterBuffer() {};
    virtual void capture(AudioBuffer<float> amps, int channel);
    virtual void capture(AudioBuffer<double> amps, int channel);
    virtual void init();
};

class MaximumAmp : public MeterBuffer
{
public:
    MaximumAmp();
    void capture(AudioBuffer<float> amps, int channel) override;
    void capture(AudioBuffer<double> amps, int channel) override;
    void init() override;
    double getMax();
private:
    juce::SpinLock mutex;
    double maxAmp;
};