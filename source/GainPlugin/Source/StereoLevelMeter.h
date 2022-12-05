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
#include "dbAnnoComponent.h"

//==============================================================================
class LevelMeter : public juce::Component
{
public:
    LevelMeter(int marginTop, int marginBottom);
    void paint(juce::Graphics&) override;
    void resized() override;
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    void init();
private:
    void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l);
    void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal);
    void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped);
    MaximumAmp maxAmp;
    int _mTop;
    int _mBottom;
    const int _peakholdTimes = 10; // number of times to leave peak 
    const float _lightheight = 15;
    const float _lightwidth = 7;
    const float _spacing = 1;
    const float _clippedheight = 10;
    const float _signalheight = 10;
    const Colour _peakColor = Colour::fromRGB(255, 0, 0);
    const Colour _ltColor = Colour::fromRGB(0, 255, 0);
    const float _lightborder = 1;
    int _nLights;
    Colour* _lightColors = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter);
};

class StereoLevelMeter : public juce::Component,
    public Timer
{
public:
    StereoLevelMeter(int minAmp, int maxAmp, int incAmp, int marginTop, int marginBottom, float leftAnnoWidth, float rightAnnoWidth);
    void resized() override;
    void capture(AudioBuffer<float> amps);
    void capture(AudioBuffer<double> amps);
    void init();
    void StereoLevelMeter::timerCallback() override;
private:

    LevelMeter leftLevelMeter;
    LevelMeter rightLevelMeter;
    dbAnnoComponent leftAnno;
    dbAnnoComponent rightAnno;
    float _leftAnnoWidth;
    float _rightAnnoWidth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoLevelMeter);
};

