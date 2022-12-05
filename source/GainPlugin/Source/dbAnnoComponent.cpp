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

dbAnnoComponent::dbAnnoComponent(int minAmp, int maxAmp, int incAmp, int marginTop, int marginBottom, float annoWidth, Justification style) :
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
    g.setColour(Colours::red);
    g.drawRect(0,0,getBounds().getWidth(),getBounds().getHeight(), 1.0);
    int width = getBounds().getWidth();
    float textWidth = width;
    float textHeight = 10.0;
    g.setColour(Colours::white);
    StringPairArray dbAnnoPos = get_db_pairs(_minAmp, _maxAmp, _incAmp, _minY, _maxY);
    g.setFont(Font("Lucinda Sans Typewriter", "Regular", 11.0));
    auto font = g.getCurrentFont();
    for (auto& key : dbAnnoPos.getAllKeys())
    {
        if (dbAnnoPos[key] != "")
        {
            g.drawText(dbAnnoPos[key], 0.0, key.getFloatValue() - (textHeight / 2.0), textWidth, textHeight, _style);
            int strWidth = font.getStringWidth(dbAnnoPos[key]);
            if (_style == Justification::left)
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
    _maxY = _marginTop;
    _minY = getBounds().getHeight() - _marginBottom;
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

float dbAnnoComponent::getYFromDb(double db)
{
    if (db <= _minAmp) return _minY;
    if (db >= _maxAmp) return _maxY;
    if (_maxAmp != _minAmp) return _minY + (int)((db - _minAmp) * ((double)(_maxY - _minY) / (_maxAmp - _minAmp)));
    return _minY;
}

StringPairArray dbAnnoComponent::get_db_pairs(int minAmp, int maxAmp, int incAmp, int minPixel, int maxPixel)
{
    StringPairArray pairs;

    for (int v = minAmp; v <= maxAmp; v++)
    {

        if (v % incAmp == 0)
        {
            addPair(pairs, "%-2.0f", (float)v, getYFromDb(v));
        }
        else
        {
            addPair(pairs, "", (float)v, getYFromDb(v));
        }
    }

    return pairs;
}