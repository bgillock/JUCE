/*
  ==============================================================================

    TwoValueAttachment.h
    Created: 26 Nov 2022 6:35:56am
    Author:  bgill

  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
namespace punch
{
    class TwoValueSliderParameterAttachment : private juce::Slider::Listener
    {
    public:
        enum ValueType {
            MinValue,
            MaxValue
        };
        /** Creates a connection between a plug-in parameters and a Two Value Slider.

            @param parameter     The parameter to use attachment
            @param slider        The Slider to use
            @param undoManager   An optional UndoManager
        */
        TwoValueSliderParameterAttachment(juce::RangedAudioParameter * parameter,
            ValueType v,
            juce::Slider& slider,
            juce::UndoManager* undoManager = nullptr);

        /** Destructor. */
        ~TwoValueSliderParameterAttachment() override;

        /** Call this after setting up your slider in the case where you need to do
            extra setup after constructing this attachment.
        */
        void sendInitialUpdate();

    private:
        void setValue(float newValue);
        void sliderValueChanged(juce::Slider*) override;

        void sliderDragStarted(juce::Slider*) override { attachment.beginGesture(); }
        void sliderDragEnded(juce::Slider*) override { attachment.endGesture(); }
        ValueType valueType;
        juce::Slider& slider;
        juce::ParameterAttachment attachment;
        bool ignoreCallbacks = false;
    };

    class JUCE_API  TwoValueSliderAttachment
    {
    public:
        TwoValueSliderAttachment(juce::AudioProcessorValueTreeState& stateToUse,
            const juce::String& parameterID1,
            const juce::String& parameterID2,
            juce::Slider& slider);

    private:
        std::unique_ptr<TwoValueSliderParameterAttachment> attachment1;
        std::unique_ptr<TwoValueSliderParameterAttachment> attachment2;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TwoValueSliderAttachment)
    };
}
