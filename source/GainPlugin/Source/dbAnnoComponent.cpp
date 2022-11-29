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

dbAnnoComponent::dbAnnoComponent(int min, int max, int inc, int marginminy, int marginmaxy, Justification s) :
    mminY(marginminy),
    mmaxY(marginmaxy),
    minY(0),
    maxY(0),
    style(s)
{
    dbmin = min;
    dbmax = max;
    dbinc = inc;
};


void dbAnnoComponent::paint(Graphics& g) 
{
    //g.setColour(Colours::red);
    //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);
    int width = getBounds().getWidth();
    float textWidth = width;
    float textHeight = 10.0;
    g.setColour(Colours::white);
    StringPairArray dbAnnoPos = get_db_pairs(dbmin, dbmax, dbinc, maxY, minY);
    g.setFont(Font("Lucinda Sans Typewriter", "Regular", 11.0));
    auto font = g.getCurrentFont();
    for (auto& key : dbAnnoPos.getAllKeys())
    {
        if (dbAnnoPos[key] != "")
        {
            g.drawText(dbAnnoPos[key], 0.0, key.getFloatValue() - (textHeight / 2.0), textWidth, textHeight, style);
            int strWidth = font.getStringWidth(dbAnnoPos[key]);
            if (style == Justification::left)
            {
                g.drawRect((float)strWidth, key.getFloatValue(), (float)width - strWidth, 1.0, 1.0);
            }
            else // must be right
            {
                g.drawRect(0.0, key.getFloatValue(), (float)width - strWidth, 1.0, 1.0);
            }
        }
        else
            g.drawRect( 0.0, key.getFloatValue(), (float)width,0.5,0.5);
    }
}

void dbAnnoComponent::resized() 
{
    maxY = mmaxY;
    minY = getBounds().getHeight() - mminY;
}


void dbAnnoComponent::addPair(StringPairArray& pairs, String format, float v, float pixel)
{

    char buffer[50];
    String annoString = "";
    if (format != "")
    {
        int n = sprintf(buffer, format.getCharPointer(), (float)v);
        annoString = buffer;
    }
    int n = sprintf(buffer, "%f", pixel);
    String pixelString = buffer;
    pairs.set(pixelString, annoString);
};

int dbAnnoComponent::getYFromDb(double db)
{
    if (dbmax != dbmin) return minY + (int)((db - dbmin) * ((double)(maxY - minY) / (dbmax - dbmin)));
    return minY;
}

StringPairArray dbAnnoComponent::get_db_pairs(int minVal, int maxVal, int increment, double minPixel, double maxPixel)
{
    

    StringPairArray pairs;

    for (int v = minVal; v <= maxVal; v++)
    {

        if (v % increment == 0)
        {
            addPair(pairs, "%-2.0f", (float)v, (float)getYFromDb(v));
        }
        else
        {
            addPair(pairs, "", (float)v, (float)getYFromDb(v));
        }
    }

    return pairs;
}