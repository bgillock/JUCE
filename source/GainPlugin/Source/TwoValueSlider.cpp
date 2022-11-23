/*
  ==============================================================================

    TwoValueSlider.cpp
    Created: 23 Nov 2022 9:12:40am
    Author:  bgill

  ==============================================================================
*/

#include <JuceHeader.h>
#include "TwoValueSlider.h"

namespace LookAndFeelHelpers
{
    static Colour createBaseColour(Colour buttonColour,
        bool hasKeyboardFocus,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) noexcept
    {
        const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
        const Colour baseColour(buttonColour.withMultipliedSaturation(sat));

        if (shouldDrawButtonAsDown)        return baseColour.contrasting(0.2f);
        if (shouldDrawButtonAsHighlighted) return baseColour.contrasting(0.1f);

        return baseColour;
    }

    static TextLayout layoutTooltipText(const String& text, Colour colour) noexcept
    {
        const float tooltipFontSize = 13.0f;
        const int maxToolTipWidth = 400;

        AttributedString s;
        s.setJustification(Justification::centred);
        s.append(text, Font(tooltipFontSize, Font::bold), colour);

        TextLayout tl;
        tl.createLayoutWithBalancedLineLengths(s, (float)maxToolTipWidth);
        return tl;
    }
}

//==============================================================================
TwoValueSliderLookAndFeel::TwoValueSliderLookAndFeel() : LookAndFeel_V2()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    //thumbImage = ImageCache::getFromMemory(BinaryData::outputonlinepngtools_png, BinaryData::outputonlinepngtools_pngSize);
}

TwoValueSliderLookAndFeel::~TwoValueSliderLookAndFeel()
{
}

void TwoValueSliderLookAndFeel::drawLinearSliderThumb(Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    const Slider::SliderStyle style, Slider& slider)
{
    jassert(style == Slider::TwoValueVertical);

    auto sliderRadius = (float)(getSliderThumbRadius(slider) - 2);

    auto knobColour = LookAndFeelHelpers::createBaseColour(slider.findColour(Slider::thumbColourId),
        slider.hasKeyboardFocus(false) && slider.isEnabled(),
        slider.isMouseOverOrDragging() && slider.isEnabled(),
        slider.isMouseButtonDown() && slider.isEnabled());

    const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

    auto sr = jmin(sliderRadius, (float)width * 0.4f);
    drawShinyButtonShape(g, 0, minSliderPos - sr,
        width, sliderRadius * 2.0,
        sliderRadius, knobColour, outlineThickness, false, false, false, false);

    g.setColour(Colour::fromRGBA(255, 127, 39, 100));
    g.fillRect(sliderRadius, maxSliderPos + sr, width - (sr * 2.0), minSliderPos - maxSliderPos - sr);
    drawShinyButtonShape(g, 0, maxSliderPos - sr,
        width, sliderRadius * 2.0,
        sliderRadius, knobColour, outlineThickness, false, false, false, false);
}
void TwoValueSliderLookAndFeel::drawShinyButtonShape(Graphics& g, float x, float y, float w, float h,
    float maxCornerSize, const Colour& baseColour, float strokeWidth,
    bool flatOnLeft, bool flatOnRight, bool flatOnTop, bool flatOnBottom)
{
    if (w <= strokeWidth * 1.1f || h <= strokeWidth * 1.1f)
        return;

    auto cs = jmin(maxCornerSize, w * 0.5f, h * 0.5f);

    Path outline;
    outline.addRoundedRectangle(x, y, w, h, cs, cs,
        !(flatOnLeft || flatOnTop),
        !(flatOnRight || flatOnTop),
        !(flatOnLeft || flatOnBottom),
        !(flatOnRight || flatOnBottom));

    ColourGradient cg(baseColour, 0.0f, y,
        baseColour.overlaidWith(Colour(0x070000ff)), 0.0f, y + h,
        false);

    cg.addColour(0.5, baseColour.overlaidWith(Colour(0x33ffffff)));
    cg.addColour(0.51, baseColour.overlaidWith(Colour(0x110000ff)));

    g.setGradientFill(cg);
    g.fillPath(outline);

    g.setColour(Colour(0x80000000));
    g.strokePath(outline, PathStrokeType(strokeWidth));
}
void TwoValueSliderLookAndFeel::drawLinearSliderBackground(Graphics& g, int x, int y, int width, int height,
    float /*sliderPos*/,
    float /*minSliderPos*/,
    float /*maxSliderPos*/,
    const Slider::SliderStyle /*style*/, Slider& slider)
{
    /*
    auto sliderRadius = (float)(getSliderThumbRadius(slider) - 2);
    auto trackColour = slider.findColour(Slider::trackColourId);
    auto gradCol1 = trackColour.overlaidWith(Colours::black.withAlpha(slider.isEnabled() ? 0.25f : 0.13f));
    auto gradCol2 = trackColour.overlaidWith(Colour(0x14000000));

    Path indent;

    if (slider.isHorizontal())
    {
        const float iy = (float)y + (float)height * 0.5f - sliderRadius * 0.5f;
        const float ih = sliderRadius;

        g.setGradientFill(ColourGradient::vertical(gradCol1, iy, gradCol2, iy + ih));

        indent.addRoundedRectangle((float)x - sliderRadius * 0.5f, iy,
            (float)width + sliderRadius, ih,
            5.0f);
    }
    else
    {
        const float ix = (float)x + (float)width * 0.5f - sliderRadius * 0.5f;
        const float iw = sliderRadius;

        g.setGradientFill(ColourGradient::horizontal(gradCol1, ix, gradCol2, ix + iw));

        indent.addRoundedRectangle(ix, (float)y - sliderRadius * 0.5f,
            iw, (float)height + sliderRadius,
            5.0f);
    }

    g.fillPath(indent);

    g.setColour(Colour(0x4c000000));
    g.strokePath(indent, PathStrokeType(0.5f));
    */
}
TwoValueSlider::TwoValueSlider(SliderStyle style, TextEntryBoxPosition textBoxPosition) : Slider(style,textBoxPosition)
{
    setLookAndFeel(&TwoValueSliderLookAndFeel);
}

TwoValueSlider::~TwoValueSlider()
{
    setLookAndFeel(nullptr);
}
