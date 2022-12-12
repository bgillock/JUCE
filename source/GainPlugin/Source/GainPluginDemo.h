/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             GainPlugin
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Gain audio plugin.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        GainProcessor

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once
#include <csignal>
#include "FaderSlider.h"
#include "TwoValueSlider.h"
#include "dbAnnoComponent.h"
#include "StereoLevelMeter.h"
#include "punch/punch.h"



//==============================================================================
class GainProcessor  : public AudioProcessor
{
public:

    //==============================================================================
    GainProcessor()
        : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo())
                                           .withOutput ("Output", AudioChannelSet::stereo())),
        state(*this, nullptr, "state",
            { std::make_unique<AudioParameterFloat>(ParameterID { "gain",  1 }, "Gain",     
                                                    NormalisableRange<float>(-25.0f, +25.0f), 0.0f),
              std::make_unique<AudioParameterFloat>(ParameterID { "targetmin",  1 }, "targetmin",
                                                    NormalisableRange<float>(-60.0f, 0.0f), -15.0f),
              std::make_unique<AudioParameterFloat>(ParameterID { "targetmax",  1 }, "targetmax",
                                                    NormalisableRange<float>(-60.0f, 0.0f), -9.0f) })
    {
        state.state.addChild({ "uiState", { { "width",  200 }, { "height", 400 } }, {} }, -1, nullptr);
    }

    //==============================================================================
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        auto gaindb = state.getParameter("gain")->getNormalisableRange().convertFrom0to1(state.getParameter("gain")->getValue());
        float gain = Decibels::decibelsToGain(gaindb);
        if (hasEditor() && getActiveEditor() != nullptr)
        {
            ((GainAudioProcessorEditor*)getActiveEditor())->captureInputMeter(buffer);
        }

        buffer.applyGain (gain);       
        if (hasEditor() && getActiveEditor() != nullptr)
        {
            ((GainAudioProcessorEditor*)getActiveEditor())->captureOutputMeter(buffer);
        }
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer&) override
    {
        auto gaindb = state.getParameter("gain")->getNormalisableRange().convertFrom0to1(state.getParameter("gain")->getValue());
        double gain = Decibels::decibelsToGain(gaindb);
        if (hasEditor() && getActiveEditor() != nullptr)
        {
            ((GainAudioProcessorEditor*)getActiveEditor())->captureInputMeter(buffer);
        }

        buffer.applyGain(gain);
        if (hasEditor() && getActiveEditor() != nullptr)
        {
            ((GainAudioProcessorEditor*)getActiveEditor())->captureOutputMeter(buffer);
        }
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GainAudioProcessorEditor (*this); }
    bool hasEditor() const override                        { return true;   }

    //==============================================================================
    const String getName() const override                  { return "Gain PlugIn"; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return "None"; }
    void changeProgramName (int, const String&) override   {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        if (auto xmlState = state.copyState().createXml())
            copyXmlToBinary(*xmlState, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
            state.replaceState(ValueTree::fromXml(*xmlState));
    }

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const auto& mainInLayout  = layouts.getChannelSet (true,  0);
        const auto& mainOutLayout = layouts.getChannelSet (false, 0);

        return (mainInLayout == mainOutLayout && (! mainInLayout.isDisabled()));
    }
    // Our plug-in's current state
    AudioProcessorValueTreeState state;

private:
    class GainAudioProcessorEditor : public AudioProcessorEditor,
        private Timer,
        private Slider::Listener,
        private Value::Listener
    {
    public:
        GainAudioProcessorEditor(GainProcessor& owner)
            : AudioProcessorEditor(owner),
            inputLevelMeter(-60, 0, 6, 19, 19, 0.0, 0.0),
            outputLevelMeter(-60, 0, 6, 19, 19, 0.0, 40.0), // 40.0 is width of target component also
            targetSlider(Slider::TwoValueVertical,Slider::NoTextBox),
            faderAnnoLeft(-25, 25, 5, 29, 25, 20.0, Justification::left),
            faderAnnoRight(-25, 25, 5, 29, 25, 20.0, Justification::right),
            gainAttachment(owner.state, "gain", gainSlider),
            targetAttachment(owner.state, "targetmin", "targetmax", targetSlider)
        {
            //raise(SIGINT); 
            inputLevelMeterLabel.setSize(40,10);
            addAndMakeVisible(inputLevelMeterLabel);
            addAndMakeVisible(inputLevelMeter);
            
            addAndMakeVisible(gainSlider);  
            gainSlider.setSliderStyle(Slider::LinearVertical);

            gainSlider.setTextBoxStyle(Slider::TextBoxAbove, false, 60, 15);
            gainSlider.setColour(Slider::ColourIds::backgroundColourId,Colours::darkgrey);
            gainSlider.addListener(this);
            gainSlider.setAlwaysOnTop(true);
            addAndMakeVisible(faderAnnoLeft);
            addAndMakeVisible(faderAnnoRight);

            outputLevelMeterLabel.setSize(40,10);
            addAndMakeVisible(outputLevelMeterLabel);
            addAndMakeVisible(outputLevelMeter);

            targetLabel.setSize(40,10);
            addAndMakeVisible(targetLabel);
            addAndMakeVisible(targetSlider);

            targetSlider.setColour(Slider::ColourIds::backgroundColourId, Colour::fromRGBA(0,0,0,0));
            targetSlider.setColour(Slider::ColourIds::thumbColourId, Colour::fromRGBA(132,121,1, 100));
            targetSlider.setColour(Slider::ColourIds::trackColourId, Colour::fromRGBA(255, 0, 0, 100));
            targetSlider.addListener(this);
         
            backgroundImage = ImageFileFormat::loadFrom(BinaryData::APIBack_PNG, BinaryData::APIBack_PNGSize);
            //addAndMakeVisible(outVUMeter);

            setResizeLimits(200, 400, 200, 400);
            setResizable(false, false);

            lastUIWidth.referTo(owner.state.state.getChildWithName("uiState").getPropertyAsValue("width", nullptr));
            lastUIHeight.referTo(owner.state.state.getChildWithName("uiState").getPropertyAsValue("height", nullptr));

            // set our component's initial size to be the last one that was stored in the filter's settings
            //setSize(lastUIWidth.getValue(), lastUIHeight.getValue());

            lastUIWidth.addListener(this);
            lastUIHeight.addListener(this);

            // start a timer which will keep our timecode display updated
            startTimerHz(10);
        }

        ~GainAudioProcessorEditor() override {}
        void paint(Graphics& g) override
        {
            //g.fillAll(Colour::fromRGB(30, 30, 30));
            g.drawImage(backgroundImage, 0, 0, getBounds().getWidth(), getBounds().getHeight(), 0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
        }
        void resized() override
        {
            // This lays out our child components...

            auto r = getLocalBounds().reduced(4);
          
            auto leftMeterArea = r.removeFromLeft(45);
            inputLevelMeterLabel.setBounds(leftMeterArea.removeFromTop(15));
            inputLevelMeter.setBounds(leftMeterArea);
            inputLevelMeter.setHeight(leftMeterArea.getHeight());
            if (inputLevelMeter.canSetRange())
                inputLevelMeter.setRange(Range<double>((double)targetSlider.getMinValue(), (double)targetSlider.getMaxValue()));

            auto sliderArea = r.removeFromLeft(64);
            sliderArea.removeFromBottom(10).removeFromTop(14); 
            gainSlider.setBounds(sliderArea);
            gainSlider.setTextBoxStyle(Slider::TextBoxAbove, false, sliderArea.getWidth(), 30);
       
            sliderArea.removeFromTop(20);
            faderAnnoLeft.setBounds(sliderArea.removeFromLeft(26));
            faderAnnoRight.setBounds(sliderArea.removeFromRight(26));

            auto rightMeterArea = r.removeFromLeft(85);
            auto rightMeterLabelArea = rightMeterArea.removeFromTop(15);
            outputLevelMeterLabel.setBounds(rightMeterLabelArea.removeFromLeft(45));
            targetLabel.setBounds(rightMeterLabelArea);
            outputLevelMeter.setBounds(rightMeterArea);
            outputLevelMeter.setHeight(rightMeterArea.getHeight());
            if (outputLevelMeter.canSetRange()) 
                outputLevelMeter.setRange(Range<double>((double)targetSlider.getMinValue(), (double)targetSlider.getMaxValue()));

            auto h = outputLevelMeter.getActualHeight();
            rightMeterArea.setHeight(h);
            rightMeterArea.removeFromTop(10);
            rightMeterArea.removeFromBottom(9);

            auto targetArea = rightMeterArea.removeFromRight(40);
            targetSlider.setBounds(targetArea); // calibrate with underlying anno!

            lastUIWidth = getWidth();
            lastUIHeight = getHeight();
        }
        
        void timerCallback() override
        {
            inputLevelMeter.repaint();
            outputLevelMeter.repaint();
        }

        template <typename FloatType>
        void captureInputMeter(AudioBuffer<FloatType> amps)
        {
            inputLevelMeter.capture(amps);
        }

        template <typename FloatType>
        void captureOutputMeter(AudioBuffer<FloatType> amps)
        {
            outputLevelMeter.capture(amps);
        }

        void sliderValueChanged(Slider* sliderThatHasChanged) override
        {
            if (sliderThatHasChanged == &gainSlider)
            {
                inputLevelMeter.clearClipped();
                outputLevelMeter.clearClipped();
            }
            if (sliderThatHasChanged == &targetSlider)
            {
                inputLevelMeter.setRange(Range<double>((double)targetSlider.getMinValue(),(double)targetSlider.getMaxValue()));
                inputLevelMeter.resized();
                outputLevelMeter.setRange(Range<double>((double)targetSlider.getMinValue(), (double)targetSlider.getMaxValue()));
                outputLevelMeter.resized();
            }
        }

    private:

        //std::unique_ptr<WaveDisplayComponent> waveDisplay;
        Label gainLabel{ {}, "Gain" };
        Label inputLevelMeterLabel{ {}, "Input"};
        Label outputLevelMeterLabel{{}, "Output"};
        Label targetLabel{ {}, "Target"};

        StereoLevelMeter inputLevelMeter;
        StereoLevelMeter outputLevelMeter;

        //dbAnnoComponent dbAnnoOut;
        dbAnnoComponent faderAnnoLeft;
        dbAnnoComponent faderAnnoRight;
        FaderSlider gainSlider;
        AudioProcessorValueTreeState::SliderAttachment gainAttachment;
        TwoValueSlider targetSlider;
        punch::TwoValueSliderAttachment targetAttachment;
        Colour backgroundColour;
        Image backgroundImage;

        // these are used to persist the UI's size - the values are stored along with the
        // filter's other parameters, and the UI component will update them when it gets
        // resized.
        Value lastUIWidth, lastUIHeight;

        //==============================================================================
        GainProcessor& getProcessor() const
        {
            return static_cast<GainProcessor&> (processor);
        }

        // called when the stored window size changes
        void valueChanged(Value& v) override
        {
            if (v.refersToSameSourceAs(lastUIHeight) ||
                v.refersToSameSourceAs(lastUIWidth))
            {
                setSize(lastUIWidth.getValue(), lastUIHeight.getValue());
            }
        }
    };
    //==============================================================================
    //AudioParameterFloat* gain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};
