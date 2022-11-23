/*
  ==============================================================================

    TwoValueSlider.h
    Created: 23 Nov 2022 9:12:40am
    Author:  bgill

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class TwoValueSliderLookAndFeel : public LookAndFeel_V2
{
public:
    TwoValueSliderLookAndFeel();
    ~TwoValueSliderLookAndFeel();
    void drawLinearSliderThumb(Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider);
    void drawLinearSliderBackground(Graphics& g, int x, int y, int width, int height,
        float /*sliderPos*/,
        float /*minSliderPos*/,
        float /*maxSliderPos*/,
        const Slider::SliderStyle /*style*/, Slider& slider);
private:
    void drawShinyButtonShape(Graphics& g, float x, float y, float w, float h,
        float maxCornerSize, const Colour& baseColour, float strokeWidth,
        bool flatOnLeft, bool flatOnRight, bool flatOnTop, bool flatOnBottom);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwoValueSliderLookAndFeel)
};

class TwoValueSlider : public juce::Slider
{
public:
    TwoValueSlider(SliderStyle style, TextEntryBoxPosition textBoxPosition);
    ~TwoValueSlider() override;

private:
    TwoValueSliderLookAndFeel TwoValueSliderLookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwoValueSlider)
};