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
FaderSlider::FaderSlider() : Slider()
{
    setLookAndFeel(&faderSliderLookAndFeel);
}

FaderSlider::~FaderSlider()
{
    setLookAndFeel(nullptr);
}

