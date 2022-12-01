/*
  ==============================================================================

    LevelMeter.cpp
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MaximumAmp.h"
#include "StereoLevelMeter.h"

StereoLevelMeter::StereoLevelMeter()
{
    //startTimerHz(100);
    addAndMakeVisible(leftLevelMeter);
    addAndMakeVisible(rightLevelMeter);
};

void StereoLevelMeter::timerCallback() 
{
    repaint();
};

void StereoLevelMeter::resized()
{
    auto r = getLocalBounds();
    leftLevelMeter.setBounds(r.removeFromLeft(r.getWidth() / 2));
    rightLevelMeter.setBounds(r);
};

void StereoLevelMeter::capture(AudioBuffer<float> amps)
{
    leftLevelMeter.capture(amps, 0);
    rightLevelMeter.capture(amps, 1);
}
void StereoLevelMeter::capture(AudioBuffer<double> amps)
{
    leftLevelMeter.capture(amps, 0);
    rightLevelMeter.capture(amps, 1);
}

void StereoLevelMeter::init()
{
    leftLevelMeter.init();
    rightLevelMeter.init();
}
void LevelMeter::paint(Graphics& g)
{
    g.setColour(Colours::red);
    g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);
    auto maxAmpDisplay = std::max(maxAmp.getMax(), -54.0);
    maxAmp.init();

    if (++maxAmp.peakTimes > peakholdTimes)
    {
        maxAmp.peakTimes = 0;
        maxAmp.peakhold = -144.0;
    }

    if (maxAmpDisplay > maxAmp.peakhold)
    {
        maxAmp.peakhold = maxAmpDisplay;
        maxAmp.peakTimes = 0;
    }

    int ysize = 15;
    auto area = getBounds().reduced(2);
    int minX = (getBounds().getWidth() / 2 - ((ysize - 4)/2));
    int minY = 20;
    int maxY = area.getHeight() - 6;

    int nlights = (maxY - minY) / ysize;
    int orangelight = (int)((float)nlights * 0.4f);
    int y = minY;
    int thislight = (int)((maxAmpDisplay / -54.0) * (float)nlights);
    int peaklight = (int)((maxAmp.peakhold / -54.0) * (float)nlights);

    for (int l = 0; l < nlights; l++)
    {
        g.setColour(Colours::white);
        g.drawRoundedRectangle(minX, y + 1, ysize - 4, ysize - 4, 5.0, 1.5);
        Colour thiscolor = Colours::black;
        if ((l >= thislight) || (l == peaklight))
        {
            if (l == 0) thiscolor = Colour::fromRGB(255, 0, 0);
            else if (l <= orangelight) thiscolor = Colours::orange;
            else thiscolor = Colour::fromRGB(0, 255, 0);
        }
        g.setColour(thiscolor);
        g.fillRoundedRectangle(minX, y + 1, ysize - 4, ysize - 4, 5.0);
        y += ysize;
    }
    maxAmp.lastlight = thislight;
    return;
};

void LevelMeter::capture(AudioBuffer<float> amps, int channel)
{
    maxAmp.capture(amps, channel);
}
void LevelMeter::capture(AudioBuffer<double> amps, int channel)
{
    maxAmp.capture(amps, channel);
}

void LevelMeter::init()
{
    maxAmp.init();
}