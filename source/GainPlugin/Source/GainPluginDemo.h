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
        return (int)(((amp - min) / (max - min)) * (double)nBins);
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
    MaximumAmp() { maxAmp = -144.0; };
    void capture(AudioBuffer<float> amps, int channel)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            float db = Decibels::gainToDecibels(amps.getRMSLevel(channel,0,amps.getNumSamples()));
            if (db > maxAmp) maxAmp = db;
        }
    }
    void capture(AudioBuffer<double> amps, int channel)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        if (lock.isLocked())
        {
            float db = Decibels::gainToDecibels(amps.getRMSLevel(channel,0,amps.getNumSamples()));
            if (db > maxAmp) maxAmp = db;
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
            maxAmp = -144.0;
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
        //startTimerHz(100);
        maxAmp = &max;
    }

    void timerCallback() override
    {
        repaint();
    }

    void paint(Graphics& g) override
    {
        g.setColour(Colours::white);
        //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);
        auto maxAmpDisplay = std::max(maxAmp->getMax(),-54.0);
        maxAmp->setMax(-144.0);

        if (++peakTimes > peakholdTimes)
        {
            peakTimes = 0;
            peakhold = -144.0;
        }
        
        if (maxAmpDisplay > peakhold)
        {
            peakhold = maxAmpDisplay;
            peakTimes = 0;
        }

        auto area = getBounds().reduced(2);
        int minX = (getBounds().getWidth()/2-6);
        int minY = 20;    
        int maxY = area.getHeight()-6;
        int ysize = 15;
        int nlights = (maxY - minY) / ysize;
        int orangelight = (int)((float)nlights * 0.4f);
        int y = minY;
        int thislight = (int)((maxAmpDisplay/-54.0) * (float)nlights);
        int peaklight = (int)((peakhold/-54.0) * (float)nlights);

        for (int l = 0; l < nlights; l++)
        {
            g.setColour(Colours::white);
            g.drawRoundedRectangle(minX, y+1, ysize-4, ysize - 4, 5.0, 1.5);
            Colour thiscolor = Colours::black;
            if ((l >= thislight) || (l == peaklight))
            {
                if (l == 0) thiscolor = Colour::fromRGB(255,0,0);
                else if (l <= orangelight) thiscolor = Colours::orange;
                else thiscolor = Colour::fromRGB(0, 255, 0);
            }
            g.setColour(thiscolor);
            g.fillRoundedRectangle(minX, y+1, ysize-4, ysize - 4,5.0);
            y += ysize;
        }
        lastlight = thislight;
        return;
    }

private:
    MaximumAmp* maxAmp;
    int lastlight = 0;
    double peakhold = 0.0;
    const int peakholdTimes = 10; // number of times to leave peak 
    int peakTimes = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

class dbAnnoComponent : public Component, private MouseListener
{
public:
    dbAnnoComponent(double min, double max, double inc)
    {
        dbmin = min;
        dbmax = max;
        dbinc = inc;
    }

    void paint(Graphics& g) override
    {
        g.setColour(Colours::white);
        //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);
        float textWidth = 30.0;
        float textHeight = 10.0;
        g.setColour(Colours::white);
        StringPairArray dbAnnoPos = get_db_pairs(dbmin, dbmax, dbinc, maxY, minY);
        for (auto& key : dbAnnoPos.getAllKeys())
        {
            g.drawText(key, minX, dbAnnoPos[key].getFloatValue() - (textHeight/2.0), textWidth, textHeight, Justification::centredLeft);
            // g.drawLine(_leftX - tickLength, dbAnnoPos[key].getFloatValue()  , _rightX, dbAnnoPos[key].getFloatValue(),1.0);
        }
    }
    void resized() override
    {
        auto area = getBounds().reduced(2);
        minX = 0;
        maxY = 20;
        width = area.getWidth();
        minY = area.getHeight() - 6;
    }
    void mouseDown(const MouseEvent& e) override
    {

    }
    void mouseDrag(const MouseEvent& e) override
    {
    }
private:
    double dbmin;
    double dbmax;
    double dbinc;
    int minY;
    int maxY;
    int width;
    int minX;

    int getYFromDb(double db)
    {
        if (dbmax != dbmin) return minY + (int)((db - dbmin) * ((double)(maxY - minY) / (dbmax - dbmin)));
        return minY;
    }
    void addPair(StringPairArray& pairs, String format, float v, float pixel)
    {
        char buffer[50];
        int n = sprintf(buffer, format.getCharPointer(), (float)v);
        String annoString = buffer;
        n = sprintf(buffer, "%f", pixel);
        String pixelString = buffer;
        pairs.set(annoString, pixelString);
    };
    
    StringPairArray get_db_pairs(double minVal, double maxVal, double increment, double minPixel, double maxPixel)
    {
        StringPairArray pairs;

        for (double v = minVal; v <= maxVal; v += increment)
        {
            if (v <= maxVal)
            {
                addPair(pairs, "%-2.0f", (float)v, (float)getYFromDb(v));
            }
        }
      
        return pairs;
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(dbAnnoComponent)
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
            { std::make_unique<AudioParameterFloat>(ParameterID { "gain",  1 }, "Gain",     
                                                    NormalisableRange<float>(-40.0f, +40.0f), 0.0f),
              std::make_unique<AudioParameterFloat>(ParameterID { "targetmin",  1 }, "targetmin",
                                                    NormalisableRange<float>(-54.0f, 0.0f), -15.0f),
              std::make_unique<AudioParameterFloat>(ParameterID { "targetmax",  1 }, "targetmax",
                                                    NormalisableRange<float>(-54.0f, 0.0f), -9.0f) })
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
        auto gaindb = state.getParameter("gain")->getNormalisableRange().convertFrom0to1(state.getParameter("gain")->getValue());
        float gain = Decibels::decibelsToGain(gaindb);
        inputMaxLeft.capture(buffer, 0);
        inputMaxRight.capture(buffer, 1);

        buffer.applyGain (gain);       
        outputMaxLeft.capture(buffer, 0);
        outputMaxRight.capture(buffer, 1);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer&) override
    {
        auto gaindb = state.getParameter("gain")->getNormalisableRange().convertFrom0to1(state.getParameter("gain")->getValue());
        double gain = Decibels::decibelsToGain(gaindb);
        inputMaxLeft.capture(buffer, 0);
        inputMaxRight.capture(buffer, 1);

        buffer.applyGain (gain);       
        outputMaxLeft.capture(buffer, 0);
        outputMaxRight.capture(buffer, 1);
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GainAudioProcessorEditor (*this,inputMaxLeft,inputMaxRight, outputMaxLeft, outputMaxRight); }
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
        GainAudioProcessorEditor(GainProcessor& owner, MaximumAmp& inLeftMaxAmp, MaximumAmp& inRightMaxAmp, MaximumAmp& outLeftMaxAmp, MaximumAmp& outRightMaxAmp)
            : AudioProcessorEditor(owner),
            inputLevelMeterLeft(inLeftMaxAmp),
            inputLevelMeterRight(inRightMaxAmp),
            outputLevelMeterLeft(outLeftMaxAmp),
            outputLevelMeterRight(outRightMaxAmp),
            targetSlider(Slider::TwoValueVertical,Slider::NoTextBox),
            dbAnnoOut(-54.0,0.0,6.0),
            gainAttachment(owner.state, "gain", gainSlider),
            targetAttachment(owner.state, "targetmin", "targetmax", targetSlider)
        {
            //raise(SIGINT); 
            inputLevelMeterLabel.setSize(40,10);
            addAndMakeVisible(inputLevelMeterLabel);
            addAndMakeVisible(inputLevelMeterLeft);
            addAndMakeVisible(inputLevelMeterRight);

            addAndMakeVisible(gainSlider);  
            gainSlider.setSliderStyle(Slider::LinearVertical);

            gainSlider.setTextBoxStyle(Slider::TextBoxAbove, false, 60, 15);
            gainSlider.setColour(Slider::ColourIds::backgroundColourId,Colours::darkgrey);
            gainSlider.setNumDecimalPlacesToDisplay(1);
            gainSlider.addListener(this);

            outputLevelMeterLabel.setSize(40,10);
            addAndMakeVisible(outputLevelMeterLabel);
            addAndMakeVisible(outputLevelMeterLeft);
            addAndMakeVisible(outputLevelMeterRight);

            targetLabel.setSize(40,10);
            addAndMakeVisible(targetLabel);
            addAndMakeVisible(dbAnnoOut);

            addAndMakeVisible(targetSlider);

            targetSlider.setColour(Slider::ColourIds::backgroundColourId, Colour::fromRGBA(0,0,0,0));
            targetSlider.setColour(Slider::ColourIds::thumbColourId, Colour::fromRGBA(255, 0, 0, 100));
            targetSlider.setColour(Slider::ColourIds::trackColourId, Colour::fromRGBA(255, 0, 0, 100));
            targetSlider.addListener(this);

            // Image myImage = ImageFileFormat::loadFrom(BinaryData::outputonlinepngtools_png, BinaryData::outputonlinepngtools_pngSize);
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
            startTimerHz(10);
        }

        ~GainAudioProcessorEditor() override {}

        //==============================================================================
        void paint(Graphics& g) override
        {
        }

        void resized() override
        {
            // This lays out our child components...

            auto r = getLocalBounds().reduced(4);
            auto annoAreaOut = r.removeFromRight(40);
            targetLabel.setBounds(annoAreaOut.removeFromTop(15));
            targetSlider.setBounds(annoAreaOut);
            dbAnnoOut.setBounds(annoAreaOut);

            targetButton.setBounds(annoAreaOut.removeFromBottom(15));
            auto leftMeterArea = r.removeFromLeft(r.getWidth()/3);
            inputLevelMeterLabel.setBounds(leftMeterArea.removeFromTop(15));
            inputLevelMeterRight.setBounds(leftMeterArea.removeFromRight(leftMeterArea.getWidth()/2));
            inputLevelMeterLeft.setBounds(leftMeterArea);

            auto sliderArea = r.removeFromLeft(r.getWidth()/2);
            gainSlider.setBounds(sliderArea);

            auto rightMeterArea = r;
            outputLevelMeterLabel.setBounds(rightMeterArea.removeFromTop(15));
            outputLevelMeterLeft.setBounds(rightMeterArea.removeFromLeft(rightMeterArea.getWidth()/2));            
            outputLevelMeterRight.setBounds(rightMeterArea);

            lastUIWidth = getWidth();
            lastUIHeight = getHeight();

        }
        
        void timerCallback() override
        {
            inputLevelMeterLeft.repaint();
            inputLevelMeterRight.repaint();
            outputLevelMeterLeft.repaint();
            outputLevelMeterRight.repaint();
        }

        void sliderValueChanged(Slider* sliderThatHasChanged) override
        {
        }

    private:

        //std::unique_ptr<WaveDisplayComponent> waveDisplay;
        Label gainLabel{ {}, "Gain" };
        Label inputLevelMeterLabel{ {}, "Input"};
        Label outputLevelMeterLabel{{}, "Output"};
        Label targetLabel{ {}, "Target"};
        TextButton targetButton{ {}, "0 db"};

        LevelMeter inputLevelMeterLeft;
        LevelMeter inputLevelMeterRight;
        LevelMeter outputLevelMeterLeft;
        LevelMeter outputLevelMeterRight;
        dbAnnoComponent dbAnnoOut;
        Slider gainSlider;
        AudioProcessorValueTreeState::SliderAttachment gainAttachment;
        Slider targetSlider;
        AudioProcessorValueTreeState::TwoValueSliderAttachment targetAttachment;
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
