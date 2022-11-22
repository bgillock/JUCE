/*
  ==============================================================================

    FaderSlider.h
    Created: 21 Nov 2022 9:26:21am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FaderSliderLookAndFeel.h"

//==============================================================================
/*
*/
class FaderSlider  : public juce::Slider
{
public:
    FaderSlider();
    ~FaderSlider() override;
  //  void mouseDown(const MouseEvent& event) override;
  //  void mouseUp(const MouseEvent& event) override;

private:
    FaderSliderLookAndFeel faderSliderLookAndFeel;
    Point<int> mousePosition;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaderSlider)
};
