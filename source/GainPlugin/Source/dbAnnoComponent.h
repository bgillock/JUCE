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
    dbAnnoComponent(int min, int max, int inc, int marginminy, int marginmaxy, Justification s);
    void paint(Graphics& g) override;
    void resized() override;

private:
    int dbmin;
    int dbmax;
    int dbinc;
    int mminY;
    int mmaxY;
    int minY;
    int maxY;
    Justification style;

    int getYFromDb(double db);
    void addPair(StringPairArray& pairs, String format, float v, float pixel);

    StringPairArray get_db_pairs(int minVal, int maxVal, int increment, double minPixel, double maxPixel);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(dbAnnoComponent)
};
