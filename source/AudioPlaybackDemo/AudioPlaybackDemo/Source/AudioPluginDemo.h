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

 name:                  AudioPluginDemo
 version:               1.0.0
 vendor:                JUCE
 website:               http://juce.com
 description:           Synthesiser audio plugin.

 dependencies:          juce_audio_basics, juce_audio_devices, juce_audio_formats,
                        juce_audio_plugin_client, juce_audio_processors,
                        juce_audio_utils, juce_core, juce_data_structures,
                        juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:             xcode_mac, vs2017, vs2022, linux_make, xcode_iphone, androidstudio

 moduleFlags:           JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             JuceDemoPluginAudioProcessor

 useLocalCopy:          1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn, pluginProducesMidiOut,
                        pluginEditorRequiresKeys
 extraPluginFormats:    AUv3

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once
#include <stdio.h>

class WaveDisplayComponent : public Component
{
public:
    WaveDisplayComponent() {
        addAndMakeVisible(canvas);
        setSize(400, 100);
    }

    // enum { height = 30 };

    void setAmps(AudioBuffer<float> &newBuffer)
    {
        floatAmps = newBuffer;
    }
    void setAmps(AudioBuffer<double>& newBuffer)
    {
        doubleAmps = newBuffer;
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

    StringPairArray get_anno_pairs(double minVal, double maxVal, double minPixel, double maxPixel, double increment, String format)
    {
        StringPairArray pairs;
        double scale = (maxPixel - minPixel) / (maxVal - minVal);

        if (increment > 0.0)
        {
            double startAnno = (int)(ceil(minVal / increment)) * increment;
            double endAnno = ((int)(floor((maxVal * 1.0001) / increment)) * increment); // rounding 
            for (double v = startAnno; v <= endAnno; v += increment)
            {
                addPair(pairs, format, (float)v, minPixel + (scale * (v - minVal)));
            }
        }
        else if (increment < 0.0)
        {
            double startAnno = (int)(floor(minVal / increment)) * increment;
            double endAnno = ((int)(ceil((maxVal * 0.9999) / increment)) * increment); // rounding 
            for (double v = startAnno; v >= endAnno; v += increment)
            {
                addPair(pairs, format, (float)v, minPixel + (scale * (v - minVal)));
            }
        }
        return pairs;
    }

    void paint(Graphics& g) override
    {
        auto r = getLocalBounds();

        g.setColour(Colours::black);
        g.fillRect(r);

        float topY = r.getY() + 15.0; 
        float leftX = r.getX() + 40.0;
        float middleY = r.getY() + (r.getHeight() / 2.0);
        float bottomY = r.getY() + r.getHeight() - 35.0;
        float rightX = r.getX() + r.getWidth() - 40.0;
        g.setColour(Colours::grey);
        g.drawRect(leftX, topY, rightX - leftX + 1, bottomY - topY + 1);
        float textWidth = 40.0;
        float textHeight = 11.0;
        float tickLength = 5.0;

        // Draw float amp annotiation
        StringPairArray ampAnnoPos = get_anno_pairs(0.0, 1.0, middleY, topY, 0.1, "%-.1f");
        for (auto& key : ampAnnoPos.getAllKeys())
        {
            g.drawText(key, leftX - textWidth - tickLength, ampAnnoPos[key].getFloatValue() - (textHeight/2.0), textWidth, textHeight, Justification::centredRight);
            g.drawLine(leftX - tickLength, ampAnnoPos[key].getFloatValue()  , rightX, ampAnnoPos[key].getFloatValue(),1.0);
        }

        StringPairArray ampAnnoNeg = get_anno_pairs(0.0, -1.0, middleY, bottomY, -0.1, "%-.1f");
        for (auto& key : ampAnnoNeg.getAllKeys())
        {
            g.drawText(key, leftX - textWidth - tickLength, ampAnnoNeg[key].getFloatValue() - (textHeight / 2.0), textWidth, textHeight, Justification::centredRight);
            g.drawLine(leftX - tickLength, ampAnnoNeg[key].getFloatValue(), rightX, ampAnnoNeg[key].getFloatValue(),1.0);
        }

        float samplesPerPixel = 1.0;
        int numSamples = (rightX - leftX) * samplesPerPixel;

        // Draw sample numbers
        StringPairArray sampleNumAnno = get_anno_pairs(0.0, numSamples, leftX, rightX, 100.0, "%3.f");
        for (auto& key : sampleNumAnno.getAllKeys())
        {
            g.drawText(key, sampleNumAnno[key].getFloatValue() - textWidth/2.0, bottomY + tickLength, textWidth, textHeight, Justification::centredRight);
            g.drawLine(sampleNumAnno[key].getFloatValue(), bottomY, sampleNumAnno[key].getFloatValue(), bottomY + tickLength,1.0);
        }
        
        int sampleRate = 48000.0;

        // Draw time scale
        StringPairArray secondsAnno = get_anno_pairs(0.0, (double)numSamples/(double)sampleRate, leftX, rightX, 0.001, "%.3f");
        for (auto& key : secondsAnno.getAllKeys())
        {
            g.drawText(key, secondsAnno[key].getFloatValue() - textWidth / 2.0, bottomY + tickLength + textHeight + 2.0, textWidth, textHeight, Justification::centredRight);
            g.drawLine(secondsAnno[key].getFloatValue(), topY, secondsAnno[key].getFloatValue(), bottomY + textHeight + 2.0 + tickLength, 1.0);
        }

        if ((floatAmps.getNumSamples() > 0) && (floatAmps.getNumChannels() > 0))
        {
            float lastAmp = floatAmps.getSample(0, 0);
            g.setColour(Colours::white);
            for (int i = 1; i < floatAmps.getNumSamples(); i++)
            {
                // just do left for now
                auto thisAmp = floatAmps.getSample(0, i);
                g.drawLine(leftX + (float)i - 1.0, middleY + (lastAmp * 50.0), leftX + (float)i, middleY + (thisAmp * 50.0), 1.0);
                lastAmp = thisAmp;
            }
        }
        if ((doubleAmps.getNumSamples() > 0) && (doubleAmps.getNumChannels() > 0))
        {
            double lastAmp = doubleAmps.getSample(0, 0);
            g.setColour(Colours::white);
            for (int i = 1; i < doubleAmps.getNumSamples(); i++)
            {
                // just do left for now
                g.drawLine(leftX + (double)i - 1.0, middleY + (lastAmp * 50.0), leftX + (double)i, middleY + (doubleAmps.getSample(0, i) * 50.0, 2.0));
            }
        }
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(5);

        canvas.setBounds(r);
    }
private:
    Component canvas;

    AudioBuffer<float> floatAmps;
    AudioBuffer<double> doubleAmps;

    void drawGrid() 
    {

    }
};

/*
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0)
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (sin (currentAngle) * level * tailOff);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        // tells the synth that this voice has stopped
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (sin (currentAngle) * level);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }


private:
    double currentAngle = 0.0;
    double angleDelta   = 0.0;
    double level        = 0.0;
    double tailOff      = 0.0;
};
*/
//==============================================================================
/** As the name suggest, this class does the actual audio processing. */
class JuceDemoPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    JuceDemoPluginAudioProcessor()
        : AudioProcessor (getBusesProperties()),
          state (*this, nullptr, "state",
                 { std::make_unique<AudioParameterFloat> (ParameterID { "gain",  1 }, "Gain",           NormalisableRange<float> (0.0f, 10.0f), 0.9f),
                   std::make_unique<AudioParameterFloat> (ParameterID { "delay", 1 }, "Delay Feedback", NormalisableRange<float> (0.0f, 1.0f), 0.5f) })
    {
        // Add a sub-tree to store the state of our UI
        state.state.addChild ({ "uiState", { { "width",  400 }, { "height", 500 } }, {} }, -1, nullptr);
    }

    ~JuceDemoPluginAudioProcessor() override = default;

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // Only mono/stereo and input/output must have same layout
        const auto& mainOutput = layouts.getMainOutputChannelSet();
        const auto& mainInput  = layouts.getMainInputChannelSet();

        // input and output layout must either be the same or the input must be disabled altogether
        if (! mainInput.isDisabled() && mainInput != mainOutput)
            return false;

        // only allow stereo and mono
        if (mainOutput.size() > 2)
            return false;

        return true;
    }

    void prepareToPlay (double newSampleRate, int /*samplesPerBlock*/) override
    {
        if (isUsingDoublePrecision())
        {
            delayBufferDouble.setSize (2, 12000);
            delayBufferFloat .setSize (1, 1);
        }
        else
        {
            delayBufferFloat .setSize (2, 12000);
            delayBufferDouble.setSize (1, 1);
        }

        reset();
    }

    void releaseResources() override
    {
        // When playback stops, you can use this as an opportunity to free up any
        // spare memory, etc.
        state.state.removeAllProperties(state.undoManager);
    }

    void reset() override
    {
        // Use this method as the place to clear any delay lines, buffers, etc, as it
        // means there's been a break in the audio's continuity.
        delayBufferFloat .clear();
        delayBufferDouble.clear();
    }

    //==============================================================================
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferFloat);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferDouble);
    }

    //==============================================================================
    bool hasEditor() const override                                   { return true; }

    AudioProcessorEditor* createEditor() override
    {
        return new JuceDemoPluginAudioProcessorEditor (*this);
    }

    //==============================================================================
    const String getName() const override                             { return "AudioPluginDemo"; }
    bool acceptsMidi() const override                                 { return false; }
    bool producesMidi() const override                                { return false; }
    double getTailLengthSeconds() const override                      { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                     { return 0; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const String getProgramName (int) override                        { return "None"; }
    void changeProgramName (int, const String&) override              {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        // Store an xml representation of our state.
        if (auto xmlState = state.copyState().createXml())
            copyXmlToBinary (*xmlState, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        // Restore our plug-in's state from the xml representation stored in the above
        // method.
        if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
            state.replaceState (ValueTree::fromXml (*xmlState));
    }

    //==============================================================================
    void updateTrackProperties (const TrackProperties& properties) override
    {
        {
            const ScopedLock sl (trackPropertiesLock);
            trackProperties = properties;
        }

        MessageManager::callAsync ([this]
        {
            if (auto* editor = dynamic_cast<JuceDemoPluginAudioProcessorEditor*> (getActiveEditor()))
                 editor->updateTrackProperties();
        });
    }

    TrackProperties getTrackProperties() const
    {
        const ScopedLock sl (trackPropertiesLock);
        return trackProperties;
    }

    class SpinLockedPosInfo
    {
    public:
        // Wait-free, but setting new info may fail if the main thread is currently
        // calling `get`. This is unlikely to matter in practice because
        // we'll be calling `set` much more frequently than `get`.
        void set (const AudioPlayHead::PositionInfo& newInfo)
        {
            const juce::SpinLock::ScopedTryLockType lock (mutex);

            if (lock.isLocked())
                info = newInfo;
        }

        AudioPlayHead::PositionInfo get() const noexcept
        {
            const juce::SpinLock::ScopedLockType lock (mutex);
            return info;
        }

    private:
        juce::SpinLock mutex;
        AudioPlayHead::PositionInfo info;
    };

    class DisplayBuffer
    {
    public:
        DisplayBuffer() {};
        DisplayBuffer(AudioBuffer<float>& buffer, const double sampleRate, const AudioPlayHead::PositionInfo startTime)
        {
            const juce::SpinLock::ScopedTryLockType lock(mutex);

            if (lock.isLocked())
            {
                _fbuffer = buffer;
                _dbuffer = AudioBuffer<double>(0, 0);
                _sampleRate = sampleRate;
                _startTime = startTime;
            }
        }

        AudioBuffer<float> getFloat() const noexcept
        {
            const juce::SpinLock::ScopedLockType lock(mutex);
            return _fbuffer;
        }
        DisplayBuffer(AudioBuffer<double>& buffer, const double sampleRate, const AudioPlayHead::PositionInfo startTime)
        {
            const juce::SpinLock::ScopedTryLockType lock(mutex);

            if (lock.isLocked())
            {
                _dbuffer = buffer;
                _fbuffer = AudioBuffer<float>(0, 0);
                _sampleRate = sampleRate;
                _startTime = startTime;
            }
        }

        AudioBuffer<double> getDouble() const noexcept
        {
            const juce::SpinLock::ScopedLockType lock(mutex);
            return _dbuffer;
        }


    private:
        juce::SpinLock mutex;
        double _sampleRate;
        AudioPlayHead::PositionInfo _startTime;
        AudioBuffer<double> _dbuffer;
        AudioBuffer<float> _fbuffer;
    };

    class SpinLockedAmps
    {
    public:
        SpinLockedAmps(const int size, const bool isUsingDoublePrecision)
        {
            if (isUsingDoublePrecision)
            {
                _doubleAmps.setSize (2, size);
                _floatAmps.setSize (1, 1);
            }
            else
            {
                _floatAmps.setSize (2, size);
                _doubleAmps.setSize (1, 1);
            }
            _nSamples = 0;
            _startIndex = 0;
            _isFloat = !isUsingDoublePrecision;
        }
        // Wait-free, but setting new info may fail if the main thread is currently
        // calling `get`. This is unlikely to matter in practice because
        // we'll be calling `set` much more frequently than `get`.

        void set(const AudioBuffer<float> &newAmps)
        {
            const juce::SpinLock::ScopedTryLockType lock(mutex);

            if (lock.isLocked() && _isFloat)
            {
                _floatAmps = newAmps;
            }
        }
        void set(const AudioBuffer<double>& newAmps)
        {
            const juce::SpinLock::ScopedTryLockType lock(mutex);

            if (lock.isLocked() && !_isFloat) 
            {
                _doubleAmps = newAmps;
            }
        }
        void add(const AudioBuffer<float>& newAmps)
        {
            const juce::SpinLock::ScopedTryLockType lock(mutex);

            if (lock.isLocked() && _isFloat)
            {
                _floatAmps = newAmps;
            }
        }
        template <typename FloatType>
        SpinLockedAmps& add(const AudioBuffer<FloatType>& newAmps)
        {
            const juce::SpinLock::ScopedTryLockType lock(mutex);
            
            if (lock.isLocked() && !_isFloat)
            {  
                int newStartIndex = _startIndex;
                int newNSamples = _nSamples;
                int bufferSize = _doubleAmps.size;

                for (int c=0; c<newAmps.getNumChannels(); c++)
                {
                    auto channelData = newAmps.getWritePointer (c);
                    int nSamples = min(newAmps.getNumSamples(),bufferSize - _nSamples)

                    for (int i=0;i<nSamples; i++)
                    {
                        _doubleAmps[n+i] = channelData[i];                     
                    }
                    newNSamples = _nSamples + nSamples;

                    int si = _startIndex;
                    if (newNSamples == bufferSize) // buffer overrun, wrap around
                    {
                        for (int i=nSamples;i<newAmps.getNumSamples(); i++) // put remaining at beg
                        {
                            _doubleAmps[si] = channelData[i];  
                            si = si + 1 % bufferSize; // wrap around                   
                        }
                        newStartIndex = si;
                    }
                }
                _startIndex = newStartIndex;
                _nSamples = newNSamples;
            }
            if (lock.isLocked() && _isFloat)
            {  
                int newStartIndex = _startIndex;
                int newNSamples = _nSamples;
                int bufferSize = _floatAmps.size;

                for (int c=0; c<newAmps.getNumChannels(); c++)
                {
                    auto channelData = newAmps.getWritePointer (c);
                    int nSamples = min(newAmps.getNumSamples(),bufferSize - _nSamples)

                    for (int i=0;i<nSamples; i++)
                    {
                        _floatAmps[_nSamples+i] = channelData[i];                     
                    }
                    newNSamples = _nSamples + nSamples;

                    int si = _startIndex;
                    if (newNSamples == bufferSize) // buffer overrun, wrap around
                    {
                        for (int i=nSamples;i<newAmps.getNumSamples(); i++) // put remaining at beg
                        {
                            _floatAmps[si] = channelData[i];  
                            si = si + 1 % bufferSize; // wrap around                   
                        }
                        newStartIndex = si;
                    }
                }
                _startIndex = newStartIndex;
                _nSamples = newNSamples;
            }
        }
        AudioBuffer<float> getFloat() const noexcept
        {
            const juce::SpinLock::ScopedLockType lock(mutex);
            AudioBuffer<float> _returnAmps(_floatAmps.getNumChannels(),_nSamples);
            if (_isFloat)
            {
                for (int c=0; c<_floatAmps.getNumChannels(); c++)
                {
                    auto channelData = _floatAmps.getReadPointer (c);
                    auto returnChannelData = _returnAmps.getWritePointer(c);
                    int rIndex = 0;
                    for (int i=_startIndex;i<_nSamples;i++)
                    {
                        returnChannelData[rIndex++] = channelData[i];
                    }
                    if (_startIndex > 0)
                    {
                        for (int i=0;i<_startIndex;i++)
                        {
                            returnChannelData[rIndex++] = channelData[i];
                        }
                    }
                }
                _startIndex = 0;
                _nSamples = 0;
                return _returnAmps;
            }
        }

        AudioBuffer<double> getDouble() const noexcept
        {
            const juce::SpinLock::ScopedLockType lock(mutex);
            AudioBuffer<double> _returnAmps(_doubleAmps.getNumChannels(),_nSamples);
            if (!_isFloat)
            {
                for (int c=0; c<_floatAmps.getNumChannels(); c++)
                {
                    auto channelData = _doubleAmps.getReadPointer (c);
                    auto returnChannelData = _returnAmps.getWritePointer(c);
                    int rIndex = 0;
                    for (int i=_startIndex;i<_nSamples;i++)
                    {
                        returnChannelData[rIndex++] = channelData[i];
                    }
                    if (_startIndex > 0)
                    {
                        for (int i=0;i<_startIndex;i++)
                        {
                            returnChannelData[rIndex++] = channelData[i];
                        }
                    }
                }
                _startIndex = 0;
                _nSamples = 0;
                return _returnAmps;
            }
        }

    private:
        juce::SpinLock mutex;
        bool _isFloat;
        int _nSamples;
        int _startIndex;
        AudioBuffer<float> _floatAmps;
        AudioBuffer<double> _doubleAmps;        
    };
    //==============================================================================
    // These properties are public so that our editor component can access them
    // A bit of a hacky way to do it, but it's only a demo! Obviously in your own
    // code you'll do this much more neatly..

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    SpinLockedPosInfo lastPosInfo;

    // this keeps a copy of the last set of amplitudes after processing
    SpinLockedAmps lastAmps(12000);
  
    bool recording = false;

    // Our plug-in's current state
    AudioProcessorValueTreeState state;

private:
    //==============================================================================
    /** This is the editor component that our filter will display. */
    class JuceDemoPluginAudioProcessorEditor  : public AudioProcessorEditor,
                                                private Timer,
                                                private Value::Listener
    {
    public:
        JuceDemoPluginAudioProcessorEditor (JuceDemoPluginAudioProcessor& owner)
            : AudioProcessorEditor (owner),
              gainAttachment       (owner.state, "gain",  gainSlider),
              delayAttachment      (owner.state, "delay", delaySlider)
        {
            // add some sliders..
            addAndMakeVisible (gainSlider);
            gainSlider.setSliderStyle (Slider::Rotary);

            addAndMakeVisible (delaySlider);
            delaySlider.setSliderStyle (Slider::Rotary);

            // add some labels for the sliders..
            gainLabel.attachToComponent (&gainSlider, false);
            gainLabel.setFont (Font (11.0f));

            delayLabel.attachToComponent (&delaySlider, false);
            delayLabel.setFont (Font (11.0f));

            addAndMakeVisible(waveDisplay);

            // add a label that will display the current timecode and status..
            addAndMakeVisible (timecodeDisplayLabel);
            timecodeDisplayLabel.setFont (Font (Font::getDefaultMonospacedFontName(), 15.0f, Font::plain));

            // set resize limits for this plug-in
            setResizeLimits (400, 200, 1024, 1024);
            setResizable (true, owner.wrapperType != wrapperType_AudioUnitv3);

            lastUIWidth .referTo (owner.state.state.getChildWithName ("uiState").getPropertyAsValue ("width",  nullptr));
            lastUIHeight.referTo (owner.state.state.getChildWithName ("uiState").getPropertyAsValue ("height", nullptr));

            // set our component's initial size to be the last one that was stored in the filter's settings
            setSize (lastUIWidth.getValue(), lastUIHeight.getValue());

            lastUIWidth. addListener (this);
            lastUIHeight.addListener (this);

            updateTrackProperties();

            // start a timer which will keep our timecode display updated
            startTimerHz (30);
        }

        ~JuceDemoPluginAudioProcessorEditor() override {}

        //==============================================================================
        void paint (Graphics& g) override
        {
            g.setColour (backgroundColour);
            g.fillAll();
        }

        void resized() override
        {
            // This lays out our child components...

            auto r = getLocalBounds().reduced (8);

            timecodeDisplayLabel.setBounds (r.removeFromTop (26));

            r.removeFromTop (20);
            auto sliderArea = r.removeFromTop (60);
            gainSlider.setBounds  (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth() / 2)));
            delaySlider.setBounds (sliderArea.removeFromLeft (jmin (180, sliderArea.getWidth())));

            auto sliderv = gainSlider.getBounds();
            r.setTop(sliderv.getBottom() + 10);

            waveDisplay.setBounds(r);
            lastUIWidth  = getWidth();
            lastUIHeight = getHeight();
        }

        void timerCallback() override
        {
            updateTimecodeDisplay (getProcessor().lastPosInfo.get());
            
            if (!getProcessor().isUsingDoublePrecision())
                waveDisplay.setAmps(getProcessor().lastAmps.getFloat());
            else 
                waveDisplay.setAmps(getProcessor().lastAmps.getDouble());

            waveDisplay.repaint();
        }

        int getControlParameterIndex (Component& control) override
        {
            if (&control == &gainSlider)
                return 0;

            if (&control == &delaySlider)
                return 1;

            return -1;
        }

        void updateTrackProperties()
        {
            auto trackColour = getProcessor().getTrackProperties().colour;
            auto& lf = getLookAndFeel();

            backgroundColour = (trackColour == Colour() ? lf.findColour (ResizableWindow::backgroundColourId)
                                                        : trackColour.withAlpha (1.0f).withBrightness (0.266f));
            repaint();
        }

    private:

        WaveDisplayComponent waveDisplay;
        Label timecodeDisplayLabel,
              gainLabel  { {}, "Throughput level:" },
              delayLabel { {}, "Delay:" };

        Slider gainSlider, delaySlider;
        AudioProcessorValueTreeState::SliderAttachment gainAttachment, delayAttachment;
        Colour backgroundColour;

        // these are used to persist the UI's size - the values are stored along with the
        // filter's other parameters, and the UI component will update them when it gets
        // resized.
        Value lastUIWidth, lastUIHeight;

        //==============================================================================
        JuceDemoPluginAudioProcessor& getProcessor() const
        {
            return static_cast<JuceDemoPluginAudioProcessor&> (processor);
        }

        //==============================================================================
        // quick-and-dirty function to format a timecode string
        static String timeToTimecodeString (double seconds)
        {
            auto millisecs = roundToInt (seconds * 1000.0);
            auto absMillisecs = std::abs (millisecs);

            return String::formatted ("%02d:%02d:%02d.%03d",
                                      millisecs / 3600000,
                                      (absMillisecs / 60000) % 60,
                                      (absMillisecs / 1000)  % 60,
                                      absMillisecs % 1000);
        }

        // quick-and-dirty function to format a bars/beats string
        static String quarterNotePositionToBarsBeatsString (double quarterNotes, AudioPlayHead::TimeSignature sig)
        {
            if (sig.numerator == 0 || sig.denominator == 0)
                return "1|1|000";

            auto quarterNotesPerBar = (sig.numerator * 4 / sig.denominator);
            auto beats  = (fmod (quarterNotes, quarterNotesPerBar) / quarterNotesPerBar) * sig.numerator;

            auto bar    = ((int) quarterNotes) / quarterNotesPerBar + 1;
            auto beat   = ((int) beats) + 1;
            auto ticks  = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));

            return String::formatted ("%d|%d|%03d", bar, beat, ticks);
        }

        // Updates the text in our position label.
        void updateTimecodeDisplay (const AudioPlayHead::PositionInfo& pos)
        {
            MemoryOutputStream displayText;

            const auto sig = pos.getTimeSignature().orFallback (AudioPlayHead::TimeSignature{});

            displayText << "[" << SystemStats::getJUCEVersion() << "]   "
                        << String (pos.getBpm().orFallback (120.0), 2) << " bpm, "
                        << sig.numerator << '/' << sig.denominator
                        << "  -  " << timeToTimecodeString (pos.getTimeInSeconds().orFallback (0.0))
                        << "  -  " << quarterNotePositionToBarsBeatsString (pos.getPpqPosition().orFallback (0.0), sig);

            if (pos.getIsRecording())
                displayText << "  (recording)";
            else if (pos.getIsPlaying())
                displayText << "  (playing)";

            timecodeDisplayLabel.setText (displayText.toString(), dontSendNotification);
        }

        void updateWaveDisplay(const AudioBuffer<float>& amps)
        {

        }
        // called when the stored window size changes
        void valueChanged (Value&) override
        {
            setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
        }
    };

    //==============================================================================
    template <typename FloatType>
    void process (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioBuffer<FloatType>& delayBuffer)
    {
        auto gainParamValue  = state.getParameter ("gain") ->getValue();
        auto delayParamValue = state.getParameter ("delay")->getValue();
        auto numSamples = buffer.getNumSamples();

        // In case we have more outputs than inputs, we'll clear any output
        // channels that didn't contain input data, (because these aren't
        // guaranteed to be empty - they may contain garbage).
        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, numSamples);

        // and now get our synth to process these midi events and generate its output.
        // synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

        // Apply our delay effect to the new output..
        applyDelay (buffer, delayBuffer, delayParamValue);

        // Apply our gain change to the outgoing data..
        applyGain (buffer, delayBuffer, gainParamValue);


        // Now ask the host for the current time so we can store it to be displayed later...
        updateCurrentTimeInfoFromHost();   
        if (recording)
        {
            // lastAmps = lastAmps.add(buffer, getSampleRate(), lastPosInfo.get().getTimeInSeconds().orFallback(0.0));
        }
        else
        {
            lastAmps.set(buffer, getSampleRate(), lastPosInfo.get().getTimeInSeconds().orFallback(0.0));
        }
    }

    template <typename FloatType>
    void applyGain (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer, float gainLevel)
    {
        ignoreUnused (delayBuffer);

        for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
            buffer.applyGain (channel, 0, buffer.getNumSamples(), gainLevel);
    }

    template <typename FloatType>
    void applyDelay (AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer, float delayLevel)
    {
        auto numSamples = buffer.getNumSamples();

        auto delayPos = 0;

        for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        {
            auto channelData = buffer.getWritePointer (channel);
            auto delayData = delayBuffer.getWritePointer (jmin (channel, delayBuffer.getNumChannels() - 1));
            delayPos = delayPosition;

            for (auto i = 0; i < numSamples; ++i)
            {
                auto in = channelData[i];
                channelData[i] += delayData[delayPos];
                delayData[delayPos] = (delayData[delayPos] + in) * delayLevel;

                if (++delayPos >= delayBuffer.getNumSamples())
                    delayPos = 0;
            }
        }

        delayPosition = delayPos;
    }

    AudioBuffer<float> delayBufferFloat;
    AudioBuffer<double> delayBufferDouble;

    int delayPosition = 0;

    CriticalSection trackPropertiesLock;
    TrackProperties trackProperties;

    void updateCurrentTimeInfoFromHost()
    {
        const auto newInfo = [&]
        {
            if (auto* ph = getPlayHead())
                if (auto result = ph->getPosition())
                    return *result;

            // If the host fails to provide the current time, we'll just use default values
            return AudioPlayHead::PositionInfo{};
        }();

        lastPosInfo.set (newInfo);
    }

    static BusesProperties getBusesProperties()
    {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                .withOutput ("Output", AudioChannelSet::stereo(), true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceDemoPluginAudioProcessor)
};
