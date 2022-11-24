/*
  ==============================================================================

    dbAnnoComponent.cpp
    Created: 24 Nov 2022 9:32:50am
    Author:  bgill

  ==============================================================================
*/

#include <JuceHeader.h>
#include "dbAnnoComponent.h"

//==============================================================================
dbAnnoComponent::dbAnnoComponent(double min, double max, double inc) :
    minX(0),
    maxY(0),
    minY(0),
    width(0)
{
    dbmin = min;
    dbmax = max;
    dbinc = inc;
};

void dbAnnoComponent::paint(Graphics& g) 
{
    g.setColour(Colours::white);
    //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);
    float textWidth = 30.0;
    float textHeight = 10.0;
    g.setColour(Colours::white);
    StringPairArray dbAnnoPos = get_db_pairs(dbmin, dbmax, dbinc, maxY, minY);
    for (auto& key : dbAnnoPos.getAllKeys())
    {
        g.drawText(key, minX, dbAnnoPos[key].getFloatValue() - (textHeight / 2.0), textWidth, textHeight, Justification::centredLeft);
        // g.drawLine(_leftX - tickLength, dbAnnoPos[key].getFloatValue()  , _rightX, dbAnnoPos[key].getFloatValue(),1.0);
    }
}
void dbAnnoComponent::resized()
{
    auto area = getBounds().reduced(2);
    minX = 0;
    maxY = 20;
    width = area.getWidth();
    minY = area.getHeight() - 6;
}

int dbAnnoComponent::getYFromDb(double db)
{
    if (dbmax != dbmin) return minY + (int)((db - dbmin) * ((double)(maxY - minY) / (dbmax - dbmin)));
    return minY;
}
void dbAnnoComponent::addPair(StringPairArray& pairs, String format, float v, float pixel)
{
    char buffer[50];
    int n = sprintf(buffer, format.getCharPointer(), (float)v);
    String annoString = buffer;
    n = sprintf(buffer, "%f", pixel);
    String pixelString = buffer;
    pairs.set(annoString, pixelString);
};

StringPairArray dbAnnoComponent::get_db_pairs(double minVal, double maxVal, double increment, double minPixel, double maxPixel)
{
    StringPairArray pairs;

    for (double v = minVal; v <= maxVal; v += increment)
    {
        if (v <= maxVal)
        {
            addPair(pairs, "%-2.0f", (float)v, (float)getYFromDb(v));
        }
    }

    return pairs;
}