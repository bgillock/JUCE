/*
  ==============================================================================

    dbAnnoComponent.h
    Created: 24 Nov 2022 9:32:50am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class dbAnnoComponent : public Component
{
public:
    dbAnnoComponent(int minAmp, int maxAmp, int incAmp, int marginTop, int marginBottom, float annoWidth, Justification style);
    void paint(Graphics& g) override;
    void resized() override;

private:

    float _minAmp;
    float _maxAmp;
    float _incAmp;
    int _marginTop;
    int _marginBottom;
    int _minY;
    int _maxY;
    float _annoWidth;
    Justification _style;

    float getYFromDb(double db);
    void addPair(StringPairArray& pairs, String format, float v, float pixel);

    StringPairArray get_db_pairs(int minAmp, int maxAmp, int incAmp, int minPixel, int maxPixel);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(dbAnnoComponent)
};
