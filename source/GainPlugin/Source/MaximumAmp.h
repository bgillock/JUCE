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
    MaximumAmp(double minAmp, double maxAmp, int nLevels, int peakHoldTimes);
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    void clear();
    float* getLevels();
    double getMinAmp() { return _minAmp; }
    double getMaxAmp() { return _maxAmp; }
    int getNLevels() { return _nLevels; }
    void setNLevels(int n);
private:
    double _minAmp;
    double _maxAmp;
    double peakAmp;
    int _nLevels;
    juce::SpinLock mutex;
    float* _levels;
    int _peakHoldTimes;  
    int lastlight = 0;
    double peakhold = 0.0;
    int peakTimes = 0;
};