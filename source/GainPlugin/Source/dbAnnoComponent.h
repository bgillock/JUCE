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
    dbAnnoComponent(double min, double max, double inc);

    void paint(Graphics& g) override;
    void resized() override;

private:
    double dbmin;
    double dbmax;
    double dbinc;
    int minY;
    int maxY;
    int width;
    int minX;

    int getYFromDb(double db);
    void addPair(StringPairArray& pairs, String format, float v, float pixel);

    StringPairArray get_db_pairs(double minVal, double maxVal, double increment, double minPixel, double maxPixel);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(dbAnnoComponent)
};
