/*
  ==============================================================================

    FaderSliderLookAndFeel.h
    Created: 21 Nov 2022 9:27:27am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class FaderSliderLookAndFeel  : public LookAndFeel_V4
{
public:
    FaderSliderLookAndFeel();
    ~FaderSliderLookAndFeel();
    void drawLinearSliderBackground(Graphics&, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const Slider::SliderStyle, Slider&) override;
    void drawLinearSlider(Graphics&, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const Slider::SliderStyle, Slider&) override; 
    void drawLabel(Graphics& g, Label& label);


private:
    Image thumbImage;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaderSliderLookAndFeel)
};
