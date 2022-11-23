/*
  ==============================================================================

    FaderSlider.cpp
    Created: 21 Nov 2022 9:26:21am
    Author:  bgill

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FaderSlider.h"

//==============================================================================
FaderSliderLookAndFeel::FaderSliderLookAndFeel() : LookAndFeel_V4()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    thumbImage = ImageCache::getFromMemory(BinaryData::outputonlinepngtools_png, BinaryData::outputonlinepngtools_pngSize);
}

FaderSliderLookAndFeel::~FaderSliderLookAndFeel()
{
}

void FaderSliderLookAndFeel::drawLinearSliderBackground(Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    const Slider::SliderStyle, Slider& slider)
{
    const float sliderRadius = (float)(getSliderThumbRadius(slider) - 2);

    const Colour trackColour(slider.findColour(Slider::trackColourId));
    const Colour gradCol1(trackColour.overlaidWith(Colour(slider.isEnabled() ? 0x13000000 : 0x09000000)));
    const Colour gradCol2(trackColour.overlaidWith(Colour(0x06000000)));
    Path indent;

    if (slider.isHorizontal())
    {
        auto iy = (float)y + (float)height * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill(ColourGradient::vertical(gradCol1, iy, gradCol2, iy + sliderRadius));

        indent.addRoundedRectangle((float)x - sliderRadius * 0.5f, iy, (float)width + sliderRadius, sliderRadius, 5.0f);
    }
    else
    {
        auto ix = (float)x + (float)width * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill(ColourGradient::horizontal(gradCol1, ix, gradCol2, ix + sliderRadius));

        indent.addRoundedRectangle(ix, (float)y - sliderRadius * 0.5f, sliderRadius, (float)height + sliderRadius, 5.0f);
    }

    g.fillPath(indent);

    g.setColour(trackColour.contrasting(0.5f));
    g.strokePath(indent, PathStrokeType(0.5f));
}

void FaderSliderLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    const Slider::SliderStyle style, Slider& slider)
{
    drawLinearSliderBackground(g, x, y, width, height,
        sliderPos, minSliderPos, maxSliderPos,
        style, slider);

    if (thumbImage.isValid())
    {
        int imgWidth = thumbImage.getWidth();
        int imgHeight = thumbImage.getHeight();

        const float centerX = width * 0.5f;

        g.drawImage(thumbImage, centerX - (float)(imgWidth / 2.0), sliderPos - (float)(imgHeight / 2.0), imgWidth, imgHeight, 0, 0, imgWidth, imgHeight);
    }
    else
    {
        static const float textPpercent = 0.35f;
        Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f, 0.5f * height, width * textPpercent, 0.5f * height);

        g.setColour(Colours::white);

        g.drawFittedText(String("No Image"), text_bounds.getSmallestIntegerContainer(), Justification::horizontallyCentred | Justification::centred, 1);
    }
}

void FaderSliderLookAndFeel::drawLabel(Graphics& g, Label& label)
{
    g.setColour(Colour(uint8(0), 0, 0, 1.0f));
    g.fillRoundedRectangle(label.getLocalBounds().removeFromTop(18).toFloat(), 3.0f);


    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font(getLabelFont(label));

        g.setColour(Colour(uint8(255), 255, 255, 1.0f));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds()).removeFromTop(15);

        auto textValue = label.getTextValue();
        auto text = String((double)textValue.getValue(), 2);
        g.drawFittedText(text, textArea, label.getJustificationType(),
            1,
            label.getMinimumHorizontalScale());

        g.setColour(Colour(uint8(255), 255, 255, 0.1f));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(Label::outlineColourId));
    }

    //g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f);
}
//==============================================================================
FaderSlider::FaderSlider() : Slider()
{
    setLookAndFeel(&faderSliderLookAndFeel);
}

FaderSlider::~FaderSlider()
{
    setLookAndFeel(nullptr);
}

