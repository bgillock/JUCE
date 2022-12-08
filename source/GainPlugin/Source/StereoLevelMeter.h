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
    LevelMeter(int marginTop, int marginBottom) :
        _mTop(marginTop),
        _mBottom(marginBottom),
        maxAmp(-54.0, 0.0, 20, 10) {};
    void paint(juce::Graphics&);
    virtual void resized() override = 0;
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    virtual void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l) = 0;
    virtual void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal) = 0;
    virtual void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped) = 0;
    virtual int getActualHeight() = 0;
    MaximumAmp maxAmp;
    int _mTop;
    int _mBottom;
    int _peakholdTimes = 0;
    int _lightheight = 0;
    int _lightwidth = 0;
    int _spacing = 0;
    int _clippedheight = 0;
    int _signalheight = 0;
    int _nLights = 0;
};

class DrawnLEDLevelMeter : public LevelMeter
{
public:
    DrawnLEDLevelMeter(int marginTop, int marginBottom);
    void resized() override;
    int getActualHeight();
    void clearClipped();
    void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l);
    void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal);
    void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped);
private:
    const Colour _peakColor = Colour::fromRGB(255, 0, 0);
    const Colour _signalColor = Colour::fromRGB(0, 255, 0);
    const float _lightborder = 1.5;
    Colour* _lightColors = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawnLEDLevelMeter);
};

class UADLevelMeter : public LevelMeter
{
public:
    UADLevelMeter(int marginTop, int marginBottom);
    void resized() override;
    int getActualHeight();
    void clearClipped();
    void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l);
    void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal);
    void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped);
private:
    const int _clippedImageOn = 3;
    const int _clippedImageOff = 0; 
    const int _signalImageOn = 5;
    const int _signalImageOff = 2;
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

