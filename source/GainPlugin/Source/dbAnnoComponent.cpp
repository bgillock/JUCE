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

dbAnnoComponent::dbAnnoComponent(float minAmp, float maxAmp, float incAmp, int marginTop, int marginBottom, float annoWidth, Justification style) :
    _style(style)
{
    _minAmp = minAmp;
    _maxAmp = maxAmp;
    _incAmp = incAmp;
    _marginTop = marginTop;
    _marginBottom = marginBottom;
    _annoWidth = annoWidth;
    _minY = 0; // init in resized
    _maxY = 0; // init in resized
};


void dbAnnoComponent::paint(Graphics& g) 
{

   // g.setColour(Colours::red);
   // g.drawRect(0,0,getBounds().getWidth(),getBounds().getHeight(), 1.0);

    int width = getBounds().getWidth();
    int textWidth = width;
    int textHeight = 10;

    StringPairArray dbAnnoPos = get_db_pairs(_minAmp, _maxAmp, _incAmp);
    g.setFont(Font("Lucinda Sans Typewriter", "Regular", 11.0f));
    auto font = g.getCurrentFont();
    for (auto& key : dbAnnoPos.getAllKeys())
    {
        if (dbAnnoPos[key] != "")
        {
            g.setColour(Colours::white);
            g.drawText(dbAnnoPos[key], 0, (int)(key.getFloatValue() - (textHeight / 2.0f)), textWidth, textHeight, _style);
            int strWidth = font.getStringWidth(dbAnnoPos[key]);
            if (_style == Justification::left)
            {
                g.drawRect((float)strWidth, key.getFloatValue(), (float)width - strWidth, 1.0f, 1.0f);
            }
            else // must be right
            {
                g.drawRect(0.0, key.getFloatValue(), (float)width - strWidth, 1.0f, 1.0f);
            }
        }
        else
        {
            g.setColour(Colours::grey);
            g.drawRect(0.0, key.getFloatValue(), (float)width, 0.5f, 0.5f);
        }
    }
}

void dbAnnoComponent::resized() 
{
    _maxY = _marginTop;
    _minY = getBounds().getHeight() - _marginBottom;
}


void dbAnnoComponent::addPair(StringPairArray& pairs, String format, float v, float pixel)
{

    char buffer[50];
    String annoString = "";
    if (format != "")
    {
        sprintf(buffer, format.getCharPointer(), (float)v);
        annoString = buffer;
    }
    sprintf(buffer, "%f", pixel);
    String pixelString = buffer;
    pairs.set(pixelString, annoString);
};

float dbAnnoComponent::getYFromDb(double db)
{
    if (db <= _minAmp) return (float)_minY;
    if (db >= _maxAmp) return (float)_maxY;
    if (_maxAmp != _minAmp) return (float)_minY + (((float)db - _minAmp) * ((float)(_maxY - _minY) / (_maxAmp - _minAmp)));
    return (float)_minY;
}

StringPairArray dbAnnoComponent::get_db_pairs(float minAmp, float maxAmp, float incAmp)
{
    StringPairArray pairs;

    for (int v = (int)minAmp; v <= (int)maxAmp; v++)
    {

        if (v % (int)incAmp == 0)
        {
            addPair(pairs, "%+2.0f", (float)v, getYFromDb(v));
        }
        else
        {
            addPair(pairs, "", (float)v, getYFromDb(v));
        }
    }

    return pairs;
}