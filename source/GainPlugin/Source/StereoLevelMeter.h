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
    LevelMeter(int marginTop, int marginBottom, int minAmp, int maxAmp, int incAmp) :
        _mTop(marginTop),
        _mBottom(marginBottom),
        maxAmp(minAmp, maxAmp, 20, 10) {};
    void paint(juce::Graphics&);
    virtual void resized() override = 0;
    void capture(AudioBuffer<float> amps, int channel);
    void capture(AudioBuffer<double> amps, int channel);
    virtual void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l) = 0;
    virtual void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal) = 0;
    virtual void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped) = 0;
    virtual int getActualHeight() = 0;
    virtual void setHeight(int height) = 0;
    virtual bool canSetRange() = 0;
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
    DrawnLEDLevelMeter(int marginTop, int marginBottom, int minAmp, int maxAmp, int incAmp);
    void resized() override;
    void setHeight(int height);
    int getActualHeight();
    void clearClipped();
    void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l);
    void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal);
    void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped);
    bool canSetRange() { return false; }
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
    UADLevelMeter(int marginTop, int marginBottom, int minAmp, int maxAmp, int incAmp);
    void resized() override;
    void setHeight(int height);
    int getActualHeight();
    void clearClipped();
    void drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l);
    void drawSignal(Graphics& g, int x, int y, int width, int height, bool signal);
    void drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped);
    bool canSetRange() { return true; }
    void setRedLevel(float level);
    void setOrangeLevel(float level);
private:
    const int _clippedImageOn = 3;
    const int _clippedImageOff = 0; 
    const int _signalImageOn = 5;
    const int _signalImageOff = 2;
    float _redLevel = -3.0;
    float _orangeLevel = -18.0;
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
    bool canSetRange();
    void setRange(Range<double> r);
    int getActualHeight();
    void setHeight(int height);
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

