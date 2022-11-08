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

class VUHistogram
{
public:
    VUHistogram(int nBins, int nBuffs, double minBin, double maxBin)
    {
        _nBins = nBins;
        _nBuffs = nBuffs;
        _minBin = minBin;
        _maxBin = maxBin;
        int** x = new int* [_nBuffs];
        for (int i = 0; i < _nBuffs; i++)
        {
            _hist[i] = new int[_nBins + 2]; // for min/max
            clearBuff(_hist[i]);
        }
        _currentBuff = 0;
    }
    void addAmps(AudioBuffer<float>& amps, int channel)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            clearBuff(_hist[_currentBuff]);
            auto channelData = amps.getReadPointer(channel);
            for (int a = 0; a < amps.getNumSamples(); a++)
            {
                _hist[_currentBuff][getBin(channelData[a], _minBin, _maxBin, _nBins)]++;
            }
        }
    }
    void getHistTotal(int* tot)
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        for (int i = 0; i <= _nBins + 1; i++)
        {
            tot[i] = 0;
            for (int b = 0; b < _nBuffs; b++)
            {
                tot[i] += _hist[b][i];
            }
        }
        return;
    }

private:
    void clearBuff(int* buff)
    {
        for (int i = 0; i < _nBins + 2; i++)
        {
            buff[i] = 0;
        }
        return;
    }
    int getBin(double amp, double min, double max, int nBins)
    {
        if (amp < min) return 0;
        if (amp > max) return nBins + 1;
        int bin = (int)(((amp - min) / (max - min)) * (double)nBins);
    }
    juce::SpinLock mutex;
    int _nBins;
    int _nBuffs;
    double _minBin;
    double _maxBin;
    int** _hist;
    int _currentBuff;
};

class MaximumAmp
{
public:
    MaximumAmp() { maxAmp = 0.0; };
    void capture(AudioBuffer<float> amps, int channel)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            auto channelData = amps.getReadPointer(channel);
            for (int a = 0; a < amps.getNumSamples(); a++)
            {
                if (abs(channelData[a]) > maxAmp) maxAmp = channelData[a];
            }
            maxAmp = channel;
        }
    }
    double getMax()
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        return maxAmp;
    }
    void setMax(double max)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            maxAmp = max;
        }
    }
    void init()
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            maxAmp = 0.0;
        }
    }
private:
    juce::SpinLock mutex;
    double maxAmp;
};
class VUComponent : public Component
{

};

class LevelMeter : public Component,
    public Timer
{
public:
    LevelMeter(MaximumAmp &max) 
    {
        startTimerHz(20);
        maxAmp = &max;
    }

    void timerCallback() override
    {
        repaint();
    }

    void paint(Graphics& g) override
    {
        auto maxAmpDisplay = maxAmp->getMax();
        auto area = getBounds().reduced(2);
        int minX = 1;
        int minY = 20;    
        int maxY = area.getHeight()-6;
        int ysize = 15;
        int nlights = (maxY - minY) / ysize;
        int orangelight = (int)((float)nlights * 0.4f);
        int y = minY;
        int thislight = (int)((1.0 - maxAmpDisplay) * (float)nlights);
        for (int l = 0; l < nlights; l++)
        {
            g.setColour(Colours::white);
            g.drawRoundedRectangle(minX, y+1, ysize-4, ysize - 4, 5.0, 1.5);
            Colour thiscolor = Colours::black;
            if (l >= thislight)
            {
                if (l <= orangelight) thiscolor = Colours::orange;
                else thiscolor = Colour::fromRGB(0, 255, 0);
            }
            g.setColour(thiscolor);
            g.fillRoundedRectangle(minX, y+1, ysize-4, ysize - 4,5.0);
            y += ysize;
        }
    }

private:
    MaximumAmp* maxAmp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

//==============================================================================
class GainProcessor  : public AudioProcessor
{
public:

    //==============================================================================
    GainProcessor()
        : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo())
                                           .withOutput ("Output", AudioChannelSet::stereo())),
        state(*this, nullptr, "state",
            { std::make_unique<AudioParameterFloat>(ParameterID { "gain",  1 }, "Gain",     NormalisableRange<float>(0.0f, 1.0f), 0.5f)})
    {
        state.state.addChild({ "uiState", { { "width",  200 }, { "height", 400 } }, {} }, -1, nullptr);
        outputMaxLeft.init();
        outputMaxRight.init();
        inputMaxLeft.init();
        inputMaxRight.init();
    }

    //==============================================================================
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        auto gain = state.getParameter("gain")->getNormalisableRange().convertFrom0to1(state.getParameter("gain")->getValue());
        //inputMaxLeft.capture(buffer, 0);
        //inputMaxRight.capture(buffer, 1);

        buffer.applyGain (gain);       
        outputMaxLeft.capture(buffer, 0);
        outputMaxRight.capture(buffer, 1);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer&) override
    {
        auto gain = state.getParameter("gain")->getNormalisableRange().convertFrom0to1(state.getParameter("gain")->getValue());
        buffer.applyGain(gain);
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GainAudioProcessorEditor (*this,inputMaxLeft,inputMaxRight); }
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
    MaximumAmp inputMaxLeft;
    MaximumAmp inputMaxRight;
    MaximumAmp outputMaxLeft;
    MaximumAmp outputMaxRight;

private:
    class GainAudioProcessorEditor : public AudioProcessorEditor,
        private Timer,
        private Slider::Listener,
        private Value::Listener
    {
    public:
        GainAudioProcessorEditor(GainProcessor& owner, MaximumAmp &leftMaxAmp, MaximumAmp& rightMaxAmp )
            : AudioProcessorEditor(owner),
            inputLevelMeterLeft(leftMaxAmp),
            inputLevelMeterRight(rightMaxAmp),
            gainAttachment(owner.state, "gain", gainSlider)
        {
           
            addAndMakeVisible(inputLevelMeterLeft);
            addAndMakeVisible(inputLevelMeterRight);

            addAndMakeVisible(gainSlider);
            gainSlider.setSliderStyle(Slider::LinearVertical);
            gainSlider.setTextBoxStyle(Slider::TextBoxAbove, false, 60, 15);
            gainSlider.setNumDecimalPlacesToDisplay(1);
            gainSlider.addListener(this);

            //addAndMakeVisible(outVUMeter);

            setResizeLimits(200, 400, 200, 1000);
            setResizable(true, owner.wrapperType != wrapperType_AudioUnitv3);

            lastUIWidth.referTo(owner.state.state.getChildWithName("uiState").getPropertyAsValue("width", nullptr));
            lastUIHeight.referTo(owner.state.state.getChildWithName("uiState").getPropertyAsValue("height", nullptr));

            // set our component's initial size to be the last one that was stored in the filter's settings
            setSize(lastUIWidth.getValue(), lastUIHeight.getValue());

            lastUIWidth.addListener(this);
            lastUIHeight.addListener(this);

            // start a timer which will keep our timecode display updated
            startTimerHz(30);
        }

        ~GainAudioProcessorEditor() override {}

        //==============================================================================
        void paint(Graphics& g) override
        {
            g.setColour(backgroundColour);
            g.fillAll();
        }

        void resized() override
        {
            // This lays out our child components...

            auto r = getLocalBounds().reduced(4);
            auto leftMeterArea = r.removeFromLeft(20);
            inputLevelMeterLeft.setBounds(leftMeterArea);
           
            auto rightMeterArea = r.removeFromLeft(20);
            inputLevelMeterRight.setBounds(rightMeterArea);

            auto sliderArea = r.removeFromLeft(60);
            gainSlider.setBounds(sliderArea);

            lastUIWidth = getWidth();
            lastUIHeight = getHeight();
        }
        void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override
        {
         
        }
        void setRange(Range<double> newRange, double newSize)
        {
            //updateCursorPosition();
            repaint();
        }
        void timerCallback() override
        {
                if (!getProcessor().isUsingDoublePrecision())
                {
                    //waveDisplay->addAmps(getProcessor().amps.getFloat());
                    //getProcessor().amps.init(12000, false, 4);
                }
                else
                {
                    //waveDisplay->addAmps(getProcessor().amps.getDouble());
                    //getProcessor().amps.init(12000, true, 4);
                }

        }

        int getControlParameterIndex(Component& control) override
        {
            if (&control == &gainSlider)
                return 0;

            return -1;
        }

        void sliderValueChanged(Slider* sliderThatHasChanged) override
        {
            
            getProcessor().inputMaxLeft.setMax(gainSlider.getValue());
            getProcessor().inputMaxRight.setMax(gainSlider.getValue() * 0.9f);
            repaint();
        }

    private:

        //std::unique_ptr<WaveDisplayComponent> waveDisplay;
        Label gainLabel{ {}, "Gain" };
        LevelMeter inputLevelMeterLeft;
        LevelMeter inputLevelMeterRight;
        Slider gainSlider;
        AudioProcessorValueTreeState::SliderAttachment gainAttachment;
        Colour backgroundColour;

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
        void valueChanged(Value&) override
        {
            setSize(lastUIWidth.getValue(), lastUIHeight.getValue());
        }
    };
    //==============================================================================
    AudioParameterFloat* gain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};
