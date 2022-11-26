/*
  ==============================================================================

    TwoValueAttachment.cpp
    Created: 26 Nov 2022 6:35:56am
    Author:  bgill

  ==============================================================================
*/

// #include "TwoValueAttachment.h"
namespace punch
{
    TwoValueSliderParameterAttachment::TwoValueSliderParameterAttachment(juce::RangedAudioParameter* param,
        ValueType v,
        juce::Slider& s,
        juce::UndoManager* um)
        : slider(s),
        valueType(v),
        attachment(*param, [this](float f) { setValue(f); }, um)
    {
        slider.valueFromTextFunction = [param](const juce::String& text) { return (double)param->convertFrom0to1(param->getValueForText(text)); };
        slider.textFromValueFunction = [param](double value) { return param->getText(param->convertTo0to1((float)value), 0); };
        slider.setDoubleClickReturnValue(true, param->convertFrom0to1(param->getDefaultValue()));

        auto range = param->getNormalisableRange();

        auto convertFrom0To1Function = [range](double currentRangeStart,
            double currentRangeEnd,
            double normalisedValue) mutable
        {
            range.start = (float)currentRangeStart;
            range.end = (float)currentRangeEnd;
            return (double)range.convertFrom0to1((float)normalisedValue);
        };

        auto convertTo0To1Function = [range](double currentRangeStart,
            double currentRangeEnd,
            double mappedValue) mutable
        {
            range.start = (float)currentRangeStart;
            range.end = (float)currentRangeEnd;
            return (double)range.convertTo0to1((float)mappedValue);
        };

        auto snapToLegalValueFunction = [range](double currentRangeStart,
            double currentRangeEnd,
            double mappedValue) mutable
        {
            range.start = (float)currentRangeStart;
            range.end = (float)currentRangeEnd;
            return (double)range.snapToLegalValue((float)mappedValue);
        };

        juce::NormalisableRange<double> newRange{ (double)range.start,
                                             (double)range.end,
                                             std::move(convertFrom0To1Function),
                                             std::move(convertTo0To1Function),
                                             std::move(snapToLegalValueFunction) };
        newRange.interval = range.interval;
        newRange.skew = range.skew;
        newRange.symmetricSkew = range.symmetricSkew;

        slider.setNormalisableRange(newRange);

        sendInitialUpdate();
        slider.valueChanged();
        slider.addListener(this);
    }

    TwoValueSliderParameterAttachment::~TwoValueSliderParameterAttachment()
    {
        slider.removeListener(this);
    }

    void TwoValueSliderParameterAttachment::sendInitialUpdate() { attachment.sendInitialUpdate(); }

    void TwoValueSliderParameterAttachment::setValue(float newValue)
    {
        const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
        valueType == ValueType::MinValue ? slider.setMinValue(newValue, juce::sendNotificationSync) : slider.setMaxValue(newValue, juce::sendNotificationSync);
    }

    void TwoValueSliderParameterAttachment::sliderValueChanged(juce::Slider*)
    {
        if (!ignoreCallbacks)
            attachment.setValueAsPartOfGesture(valueType == ValueType::MinValue ? (float)slider.getMinValue() : (float)slider.getMaxValue());
    }

    TwoValueSliderAttachment::TwoValueSliderAttachment(juce::AudioProcessorValueTreeState& stateToUse,
        const juce::String& parameterID1,
        const juce::String& parameterID2,
        juce::Slider& slider)
        : attachment1(std::make_unique<TwoValueSliderParameterAttachment>(stateToUse.getParameter(parameterID1),
            TwoValueSliderParameterAttachment::ValueType::MinValue,
            slider,
            stateToUse.undoManager)),
        attachment2(std::make_unique<TwoValueSliderParameterAttachment>(stateToUse.getParameter(parameterID2),
            TwoValueSliderParameterAttachment::ValueType::MaxValue,
            slider,
            stateToUse.undoManager))
    {
    }
}