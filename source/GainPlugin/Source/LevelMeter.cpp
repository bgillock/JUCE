/*
  ==============================================================================

    LevelMeter.cpp
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MaximumAmp.h"
#include "LevelMeter.h"

LevelMeter::LevelMeter(MaximumAmp& max)
{
    //startTimerHz(100);
    maxAmp = &max;
};

void LevelMeter::timerCallback() 
{
    repaint();
};

void LevelMeter::paint(Graphics& g)
{
    //g.setColour(Colours::red);
    //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);
    auto maxAmpDisplay = std::max(maxAmp->getMax(), -54.0);
    maxAmp->setMax(-144.0);

    if (++peakTimes > peakholdTimes)
    {
        peakTimes = 0;
        peakhold = -144.0;
    }

    if (maxAmpDisplay > peakhold)
    {
        peakhold = maxAmpDisplay;
        peakTimes = 0;
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
    int peaklight = (int)((peakhold / -54.0) * (float)nlights);

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
    lastlight = thislight;
    return;
};

