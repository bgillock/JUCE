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
    LevelMeter() {};
    virtual void paint(juce::Graphics&) override = 0;
    virtual void resized() override = 0;
    virtual void capture(AudioBuffer<float> amps, int channel) = 0;
    virtual void capture(AudioBuffer<double> amps, int channel) = 0;
    virtual int getActualHeight() = 0;
};

class DrawnLEDLevelMeter : public LevelMeter
{
public:
    DrawnLEDLevelMeter(int marginTop, int marginBottom);
    void paint(juce::Graphics&) override;
    void resized() override;
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    int getActualHeight();
    void clearClipped();
private:
    void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l);
    void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal);
    void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped);
    MaximumAmp maxAmp;
    int _mTop;
    int _mBottom;
    const int _peakholdTimes = 10; // number of times to leave peak 
    const float _lightheight = 9;
    const float _lightwidth = 17;
    const float _spacing = 1;
    const float _clippedheight = 10;
    const float _signalheight = 10;
    const Colour _peakColor = Colour::fromRGB(255, 0, 0);
    const Colour _signalColor = Colour::fromRGB(0, 255, 0);
    const float _lightborder = 1.5;
    int _nLights;
    Colour* _lightColors = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawnLEDLevelMeter);
};

class UADLevelMeter : public LevelMeter
{
public:
    UADLevelMeter(int marginTop, int marginBottom);
    void paint(juce::Graphics&) override;
    void resized() override;
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    int getActualHeight();
    void clearClipped();
private:
    void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l);
    void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal);
    void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped);
    MaximumAmp maxAmp;
    int _mTop;
    int _mBottom;
    const int _peakholdTimes = 10; // number of times to leave peak 
    const float _lightheight = 16;
    const float _lightwidth = 16;
    const float _spacing = 2;
    const float _clippedheight = 16;
    const float _signalheight = 16;
    const int _clippedImageOn = 3;
    const int _clippedImageOff = 0; 
    const int _signalImageOn = 5;
    const int _signalImageOff = 2;
    int _nLights;
    int* _lightImageIndexes = nullptr;
    Image _lightImages;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UADLevelMeter);
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
    void clearClipped();
    void StereoLevelMeter::timerCallback() override;
private:

    UADLevelMeter leftLevelMeter;
    UADLevelMeter rightLevelMeter;
    dbAnnoComponent leftAnno;
    dbAnnoComponent rightAnno;
    float _leftAnnoWidth;
    float _rightAnnoWidth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoLevelMeter);
};

