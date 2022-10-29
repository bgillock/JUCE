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

static PerformanceCounter spinlockedampsadd("SpinLockedAmps.add");
static PerformanceCounter spinlockedampsget("SpinLockedAmps.get");
static PerformanceCounter drawampsanno("DrawAmpsAnno");
static PerformanceCounter drawtimeanno("DrawTimeAnno");
static PerformanceCounter drawamps("DrawAmps");

class SpinLockedAmps 
{
public:
    void init(const int size, const bool isUsingDoublePrecision, int nchannels)
    {
        if (isUsingDoublePrecision)
        {
            _doubleAmps.setSize(nchannels, size);
            _floatAmps.setSize(1, 1);
        }
        else
        {
            _floatAmps.setSize(nchannels, size);
            _doubleAmps.setSize(1, 1);
        }
        _nSamples = 0;
        _startIndex = 0;
        _isFloat = !isUsingDoublePrecision;
        _bufferSize = size;
        _nChannels = nchannels;
    }
    // Wait-free, but setting new info may fail if the main thread is currently
    // calling `get`. This is unlikely to matter in practice because
    // we'll be calling `set` much more frequently than `get`.

    void set(const AudioBuffer<float>& newAmps)
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

    template <typename FloatType>
    void add(AudioBuffer<FloatType>& newAmps)
    {
        spinlockedampsadd.start();

        const juce::SpinLock::ScopedTryLockType lock(mutex);

        if (lock.isLocked() && (_isFloat))
        {
            int newStartIndex = _startIndex;
            int newNSamples = _nSamples;
            int bufferSize = _floatAmps.getNumSamples();

            for (int c = 0; c < newAmps.getNumChannels(); c++)
            {
                auto newChannelData = newAmps.getReadPointer(c);
                auto floatChannelData = _floatAmps.getWritePointer(c);

                int nSamples = std::min(newAmps.getNumSamples(), bufferSize - _nSamples);

                for (int i = 0; i < nSamples; i++)
                {
                    jassert(_nSamples + i < _floatAmps.getNumSamples());
                    floatChannelData[_nSamples + i] = newChannelData[i];
                }
                newNSamples = _nSamples + nSamples;

                int si = _startIndex;
                if (newNSamples == bufferSize) // buffer overrun, wrap around
                {
                    for (int i = nSamples; i < newAmps.getNumSamples(); i++) // put remaining at beg
                    {
                        jassert(si < _floatAmps.getNumSamples());
                        floatChannelData[si] = newChannelData[i];
                        si = (si + 1) % bufferSize; // wrap around                   
                    }
                    newStartIndex = si;
                }
            }
            _startIndex = newStartIndex;
            _nSamples = newNSamples;
            if (_nSamples > _maxNSamples) _maxNSamples = _nSamples;
        }
        if (lock.isLocked() && !_isFloat)
        {
            int newStartIndex = _startIndex;
            int newNSamples = _nSamples;
            int bufferSize = _doubleAmps.getNumSamples();

            for (int c = 0; c < newAmps.getNumChannels(); c++)
            {
                auto newChannelData = newAmps.getReadPointer(c);
                auto doubleChannelData = _doubleAmps.getWritePointer(c);

                int nSamples = std::min(newAmps.getNumSamples(), bufferSize - _nSamples);

                for (int i = 0; i < nSamples; i++)
                {
                    jassert(_nSamples + i < _doubleAmps.getNumSamples());
                    doubleChannelData[_nSamples + i] = newChannelData[i];
                }
                newNSamples = _nSamples + nSamples;

                int si = _startIndex;
                if (newNSamples == bufferSize) // buffer overrun, wrap around
                {
                    for (int i = nSamples; i < newAmps.getNumSamples(); i++) // put remaining at beg
                    {
                        jassert(si < _doubleAmps.getNumSamples());
                        doubleChannelData[si] = newChannelData[i];
                        si = (si + 1) % bufferSize; // wrap around                   
                    }
                    newStartIndex = si;
                }
            }
            _startIndex = newStartIndex;
            _nSamples = newNSamples;
            if (_nSamples > _maxNSamples) _maxNSamples = _nSamples;
        }
        spinlockedampsadd.stop();
    }
    AudioBuffer<float> getFloat() noexcept
    {
        spinlockedampsget.start();
        const juce::SpinLock::ScopedLockType lock(mutex);
        AudioBuffer<float> _returnAmps(_floatAmps.getNumChannels(), _nSamples);
        jassert(_isFloat);
        for (int c = 0; c < _floatAmps.getNumChannels(); c++)
        {
            auto channelData = _floatAmps.getReadPointer(c);
            auto returnChannelData = _returnAmps.getWritePointer(c);
            int rIndex = 0;
            for (int i = _startIndex; i < _nSamples; i++)
            {
                returnChannelData[rIndex++] = channelData[i];
            }
            if (_startIndex > 0)
            {
                for (int i = 0; i < _startIndex; i++)
                {
                    returnChannelData[rIndex++] = channelData[i];
                }
            }
        }
        //_startIndex = 0;
        //_nSamples = 0;
        spinlockedampsget.stop();
        return _returnAmps;
    }

    AudioBuffer<double> getDouble() noexcept
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        AudioBuffer<double> _returnAmps(_doubleAmps.getNumChannels(), _nSamples);
        jassert(!_isFloat);

        for (int c = 0; c < _floatAmps.getNumChannels(); c++)
        {
            auto channelData = _doubleAmps.getReadPointer(c);
            auto returnChannelData = _returnAmps.getWritePointer(c);
            int rIndex = 0;
            for (int i = _startIndex; i < _nSamples; i++)
            {
                returnChannelData[rIndex++] = channelData[i];
            }
            if (_startIndex > 0)
            {
                for (int i = 0; i < _startIndex; i++)
                {
                    returnChannelData[rIndex++] = channelData[i];
                }
            }
        }

        //_startIndex = 0;
        //_nSamples = 0;
        return _returnAmps;
    }
    bool isFloat() {
        return _isFloat;
    }
    int getCurrentSize() {
        return _nSamples;
    }
    int getMaxSize()
    {
        return _bufferSize;
    }
    int getNChannels()
    {
        return _nChannels;
    }
private:
    juce::SpinLock mutex;
    bool _isFloat;
    int _nSamples;
    int _startIndex;
    int _maxNSamples = 0;
    int _nChannels = 0;
    int _bufferSize = 0;
    AudioBuffer<float> _floatAmps;
    AudioBuffer<double> _doubleAmps;
};

class WaveDisplayComponent : public Component
{
public:
    WaveDisplayComponent(AudioProcessorValueTreeState *state, bool isUsingDoublePrecision) {
        setSize(400, 100);
        _state = state;
        _amps.init(48000.0 * 5.0, isUsingDoublePrecision, 4);
        _startSample = 0;
        _samplesPerPixel = 10.0;
        _viewSizePixels = 400;
        _vscale = 0.5;
    }
    void restartBuffer()
    {
        _amps.init(_amps.getMaxSize(), !_amps.isFloat(), _amps.getNChannels());
    }

    void setSamplesPerPixel(double spp)
    {
        _samplesPerPixel = std::min<double>(spp, 100.0);
        _samplesPerPixel = std::max<double>(_samplesPerPixel, 0.1);
    }

    void setStartSample(int startSample)
    {
        jassert(startSample >= 0.0);
        Range<double> sampleRange = getViewRange();
        if (startSample > getBufferSize() - sampleRange.getLength() - 1)
            startSample = std::max<int>(getBufferSize() - sampleRange.getLength() - 1, 0);
        _startSample = startSample;
        repaint();
    }

    void setVerticalScale(double scale)
    {
        _vscale = std::max(scale, 0.1);
        repaint();
    }

    int getBufferSize()
    {
        return _amps.getCurrentSize();
    }

    Range<double> getViewRange()
    {
        double endSample = _startSample + ((double)_viewSizePixels * _samplesPerPixel);
        if (endSample >= getBufferSize()) endSample = getBufferSize() - 1.0;
        return Range<double>(_startSample, endSample);
    }
    // enum { height = 30 };
    void addAmps(AudioBuffer<float>& buffer)
    {
        _amps.add(buffer);
    }
    void addAmps(AudioBuffer<double>& buffer)
    {
        _amps.add(buffer);
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

    void drawFloatAmps(Graphics& g, int startSample, double scale, int numSamples, int channel,
                       int leftX, int middleY, double samplesPerPixel, Colour color, bool minmax = false) // avg = !minmax
    {
        if ((_amps.getCurrentSize() > 0) && (_amps.isFloat()))
        {
            AudioBuffer<float> floatAmps = _amps.getFloat();
            float lastAmp = floatAmps.getSample(channel, startSample) * scale;
            int n = std::min(floatAmps.getNumSamples(), startSample + numSamples);
            g.setColour(color);

            if (samplesPerPixel < 1.5)
            {
                // draw lines from sample to sample
                for (int i = startSample; i < n; i++)
                {
                    // just do left for now
                    auto thisAmp = floatAmps.getSample(channel, i) * scale;
                    g.drawLine(leftX + ((float)(i - startSample - 1.0) / samplesPerPixel), middleY + (lastAmp), leftX + ((float)(i - startSample) / samplesPerPixel), middleY + (thisAmp), 1.0);
                    lastAmp = thisAmp;
                }
            }
            else
            {
                // draw 1 pix wide vertical rectangles from min-max within pixel
                int thisX = leftX; 
                int endX = leftX + (int)((double)n / samplesPerPixel) + 1;

                if (minmax)
                {
                    Range<int> mm = Range<int>((int)lastAmp, (int)lastAmp);
                    for (int i = startSample + 1; i < n; i++)
                    {
                        int x = leftX + ((float)(i - startSample - 1.0) / samplesPerPixel);
                        auto thisAmp = floatAmps.getSample(channel, i) * scale;
                        if (x > thisX)
                        {
                            g.drawRect(Rectangle<float>(thisX, middleY + mm.getStart(), 1, mm.getLength()));
                            thisX = x;
                            mm = Range<int>((int)lastAmp, (int)lastAmp);
                        }
                        mm = mm.getUnionWith((int)thisAmp);
                        lastAmp = thisAmp;
                    }
                    g.drawRect(Rectangle<float>(thisX, middleY + mm.getStart(), 1, std::max(mm.getLength(), 1)));
                }
                else
                {
                    StatisticsAccumulator<float> avg = StatisticsAccumulator<float>();
                    avg.addValue(lastAmp);
                    const double pps = 1.0 / samplesPerPixel;
                    int i = startSample + 1;
                    double s = 0.0;
                    int x = leftX + s;
                    while ((x == thisX) && (i < n))
                    {
                        auto thisAmp = floatAmps.getSample(channel, i) * scale;
                        avg.addValue(thisAmp);
                        i++;
                        s += pps;
                        x = leftX + s;
                    };
                    auto lastAvg = avg.getAverage();
                    avg.reset();
                    thisX = x;
                    while (i < n)
                    {
                        int x = leftX + s;
                        auto thisAmp = floatAmps.getSample(channel, i) * scale;
                        if (x > thisX)
                        {
                            g.drawLine(thisX-1, middleY + (int)lastAvg, thisX, middleY + (int)avg.getAverage());
                            thisX = x;
                            lastAvg = avg.getAverage();
                            avg.reset();
                        }
                        avg.addValue(thisAmp);
                        i++;
                        s += pps;
                    }
                    if (avg.getCount() > 0) g.drawLine(thisX - 1, middleY + (int)lastAvg, thisX, middleY + (int)avg.getAverage());
                }
            }
        }
    }
    void paint(Graphics& g) override
    {
        _vscale = _state->getParameter("vscale")->getNormalisableRange().convertFrom0to1(_state->getParameter("vscale")->getValue());
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
        drawampsanno.start();
        // Draw float amp annotiation
        StringPairArray ampAnnoPos = get_anno_pairs(0.0, _vscale, middleY, topY, 0.1, "%-.1f");
        for (auto& key : ampAnnoPos.getAllKeys())
        {
            g.drawText(key, leftX - textWidth - tickLength, ampAnnoPos[key].getFloatValue() - (textHeight/2.0), textWidth, textHeight, Justification::centredRight);
            g.drawLine(leftX - tickLength, ampAnnoPos[key].getFloatValue()  , rightX, ampAnnoPos[key].getFloatValue(),1.0);
        }

        StringPairArray ampAnnoNeg = get_anno_pairs(0.0, -_vscale, middleY, bottomY, -0.1, "%-.1f");
        for (auto& key : ampAnnoNeg.getAllKeys())
        {
            g.drawText(key, leftX - textWidth - tickLength, ampAnnoNeg[key].getFloatValue() - (textHeight / 2.0), textWidth, textHeight, Justification::centredRight);
            g.drawLine(leftX - tickLength, ampAnnoNeg[key].getFloatValue(), rightX, ampAnnoNeg[key].getFloatValue(),1.0);
        }
        drawampsanno.stop();

        int startSample = _startSample;
        float samplesPerPixel = _samplesPerPixel;

        
        double scale = (double)(middleY-topY)/(_vscale);
        _viewSizePixels = rightX - leftX + 1;
        int numSamples = (rightX - leftX) * samplesPerPixel;

        drawtimeanno.start();
        // Draw sample numbers
        StringPairArray sampleNumAnno = get_anno_pairs(startSample, startSample+numSamples, leftX, rightX, 1000.0, "%5.f");
        for (auto& key : sampleNumAnno.getAllKeys())
        {
            g.drawText(key, sampleNumAnno[key].getFloatValue() - textWidth/2.0, bottomY + tickLength, textWidth, textHeight, Justification::centredRight);
            g.drawLine(sampleNumAnno[key].getFloatValue(), bottomY, sampleNumAnno[key].getFloatValue(), bottomY + tickLength,1.0);
        }
        
        int sampleRate = 48000.0;

        // Draw time scale
        StringPairArray secondsAnno = get_anno_pairs((double)startSample/(double)sampleRate, (double)(startSample+numSamples)/(double)sampleRate, leftX, rightX, 0.01, "%.2f");
        for (auto& key : secondsAnno.getAllKeys())
        {
            g.drawText(key, secondsAnno[key].getFloatValue() - textWidth / 2.0, bottomY + tickLength + textHeight + 2.0, textWidth, textHeight, Justification::centredRight);
            g.drawLine(secondsAnno[key].getFloatValue(), topY, secondsAnno[key].getFloatValue(), bottomY + textHeight + 2.0 + tickLength, 1.0);
        }
        drawtimeanno.stop();

        drawamps.start();
        // display pre (channel 0)
        drawFloatAmps(g, startSample, scale, numSamples, 0, leftX, middleY, samplesPerPixel, Colours::red);

        // display post (channel 2)
        drawFloatAmps(g, startSample, scale, numSamples, 2, leftX, middleY, samplesPerPixel, Colours::white);

        drawamps.stop();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(5);
    }
private:

    SpinLockedAmps _amps;
    int _startSample;
    double _samplesPerPixel;
    int _viewSizePixels;
    double _vscale;
    AudioProcessorValueTreeState *_state;
};

//==============================================================================
/** As the name suggest, this class does the actual audio processing. */
class JuceDemoPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    JuceDemoPluginAudioProcessor()
        : AudioProcessor (getBusesProperties()),
          state (*this, nullptr, "state",
                 { std::make_unique<AudioParameterFloat> (ParameterID { "threshold",  1 }, "Threshold",     NormalisableRange<float> (0.0f, 1.0f), 0.6f),
                   std::make_unique<AudioParameterFloat> (ParameterID { "release", 1 }, "Release",          NormalisableRange<float> (0.0f, 250.0f), 25.0f),
                   std::make_unique<AudioParameterFloat> (ParameterID { "vscale",  1 }, "Vertical Scale",    NormalisableRange<float>(0.1f, 1.0f), 0.5f),
                   std::make_unique<AudioParameterFloat> (ParameterID { "hscale",  1 }, "Horizontal Scale",  NormalisableRange<float>(0.1f, 100.0f), 2.0f) })
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
            captureBufferDouble.setSize(4, 12000);
            captureBufferFloat.setSize(1, 1);
        }
        else
        {
            captureBufferFloat.setSize(4, 12000);            
            captureBufferDouble.setSize(1, 1);
        }

        amps.init(12000, isUsingDoublePrecision(), 4);

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
        captureBufferFloat.clear();
        captureBufferDouble.clear();
    }

    //==============================================================================
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());
        process (buffer, midiMessages);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());
        process (buffer, midiMessages);
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

    void setRecording(bool recording)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);

        if (lock.isLocked())
            _recording = recording;
    }
    bool isRecording()
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        return _recording;
    }

    //==============================================================================
    // These properties are public so that our editor component can access them
    // A bit of a hacky way to do it, but it's only a demo! Obviously in your own
    // code you'll do this much more neatly..

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    SpinLockedPosInfo lastPosInfo;

    SpinLockedAmps amps;

    // Our plug-in's current state
    AudioProcessorValueTreeState state;

private:
    //==============================================================================
    /** This is the editor component that our filter will display. */
    class JuceDemoPluginAudioProcessorEditor : public AudioProcessorEditor,
        private Timer,
        private ScrollBar::Listener,
        private Slider::Listener,
        private Button::Listener,
        private Value::Listener
    {
    public:
        JuceDemoPluginAudioProcessorEditor(JuceDemoPluginAudioProcessor& owner)
            : AudioProcessorEditor(owner),
            thresholdAttachment(owner.state, "threshold", thresholdSlider),
            releaseAttachment(owner.state, "release", releaseSlider),
            vscaleAttachment(owner.state, "vscale", vscaleSlider),
            hscaleAttachment(owner.state, "hscale", hscaleSlider)
        {
            // add some sliders..
            addAndMakeVisible(thresholdSlider);
            thresholdSlider.setSliderStyle(Slider::Rotary);

            addAndMakeVisible(releaseSlider);
            releaseSlider.setSliderStyle(Slider::Rotary);

            // add some labels for the sliders..
            thresholdLabel.attachToComponent(&thresholdSlider, false);
            thresholdLabel.setFont(Font(11.0f));

            releaseLabel.attachToComponent(&releaseSlider, false);
            releaseLabel.setFont(Font(11.0f));

            waveDisplay = std::make_unique<WaveDisplayComponent>(&getProcessor().state,getProcessor().isUsingDoublePrecision());
            addAndMakeVisible(*waveDisplay);

            addAndMakeVisible(hscrollbar);
            hscrollbar.setRangeLimits(Range<double>(0.0, 200.0));
            hscrollbar.setAutoHide(false);
            hscrollbar.addListener(this);

            addAndMakeVisible(vscaleSlider);
            vscaleSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
            vscaleSlider.setValue(0.5);
            vscaleSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 15);
            vscaleSlider.setNumDecimalPlacesToDisplay(1);
            vscaleSlider.addListener(this);

            addAndMakeVisible(recordbutton);
            recordbutton.setOnColours(Colours::red, Colours::red, Colours::red);
            recordbutton.setClickingTogglesState(true);
            recordbutton.shouldUseOnColours(true);
            recordbutton.addListener(this);

            // set resize limits for this plug-in
            setResizeLimits(400, 200, 1024, 1024);
            setResizable(true, owner.wrapperType != wrapperType_AudioUnitv3);

            lastUIWidth.referTo(owner.state.state.getChildWithName("uiState").getPropertyAsValue("width", nullptr));
            lastUIHeight.referTo(owner.state.state.getChildWithName("uiState").getPropertyAsValue("height", nullptr));

            // set our component's initial size to be the last one that was stored in the filter's settings
            setSize(lastUIWidth.getValue(), lastUIHeight.getValue());

            lastUIWidth.addListener(this);
            lastUIHeight.addListener(this);

            updateTrackProperties();

            // start a timer which will keep our timecode display updated
            startTimerHz(100);
        }

        ~JuceDemoPluginAudioProcessorEditor() override {}

        //==============================================================================
        void paint(Graphics& g) override
        {
            g.setColour(backgroundColour);
            g.fillAll();
        }

        void resized() override
        {
            // This lays out our child components...

            auto r = getLocalBounds().reduced(8);

            r.removeFromTop(20);
            auto sliderArea = r.removeFromTop(60);
            thresholdSlider.setBounds(sliderArea.removeFromLeft(jmin(180, sliderArea.getWidth() / 2)));
            releaseSlider.setBounds(sliderArea.removeFromLeft(jmin(180, sliderArea.getWidth())));

            auto sliderv = thresholdSlider.getBounds();
            r.setTop(sliderv.getBottom() + 10);
            
            auto harea = r.removeFromBottom(20);
            recordbutton.setBounds(harea.removeFromRight(20).reduced(3));
            Path p;
            p.addRoundedRectangle(3, 3, 8, 8, 3);
            recordbutton.setShape(p, true, true, true);

            hscrollbar.setBounds(harea.reduced(3));
            vscaleSlider.setBounds(r.removeFromRight(50).reduced(3));
            waveDisplay->setBounds(r.reduced(3));

            lastUIWidth = getWidth();
            lastUIHeight = getHeight();
        }
        void setRange(Range<double> newRange)
        {
            visibleRange = newRange;
            hscrollbar.setRangeLimits(Range<double>(0, (double)waveDisplay->getBufferSize()));
            int bsize = waveDisplay->getBufferSize();
            hscrollbar.setCurrentRange(visibleRange);

            //updateCursorPosition();
            repaint();
        }
        void timerCallback() override
        {
            if (getProcessor().isRecording())
            {
                if (!getProcessor().isUsingDoublePrecision())
                {
                    waveDisplay->addAmps(getProcessor().amps.getFloat());
                    getProcessor().amps.init(12000, false, 4);
                }
                else
                {
                    waveDisplay->addAmps(getProcessor().amps.getDouble());
                    getProcessor().amps.init(12000, true, 4);
                }

                waveDisplay->repaint();
                setRange(waveDisplay->getViewRange());
            }
        }

        int getControlParameterIndex(Component& control) override
        {
            if (&control == &thresholdSlider)
                return 0;

            if (&control == &releaseSlider)
                return 1;

            if (&control == &vscaleSlider)
                return 2;

            if (&control == &hscaleSlider)
                return 3;
            return -1;
        }

        void updateTrackProperties()
        {
            auto trackColour = getProcessor().getTrackProperties().colour;
            auto& lf = getLookAndFeel();

            backgroundColour = (trackColour == Colour() ? lf.findColour(ResizableWindow::backgroundColourId)
                : trackColour.withAlpha(1.0f).withBrightness(0.266f));
            repaint();
        }
        void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
        {
            if (scrollBarThatHasMoved == &hscrollbar)
                waveDisplay->setStartSample((int)hscrollbar.getCurrentRangeStart());
            //        setRange(visibleRange.movedToStartAt(newRangeStart));
            //if (scrollBarThatHasMoved == &vscrollbar)
            //    waveDisplay->setVerticalScale(vscrollbar.getCurrentRangeStart());
        }
        void sliderValueChanged(Slider *sliderThatHasChanged) override
        {
            repaint();
        }

        void buttonClicked(Button* button) override
        {
            if (button == &recordbutton)
            {

            }
        }
        void buttonStateChanged(Button* button) override 
        {
            if (button == &recordbutton)
            {
                if (recordbutton.getToggleState() && !getProcessor().isRecording())
                {
                    getProcessor().setRecording(true);
                    waveDisplay->restartBuffer();
                }
                else if (!recordbutton.getToggleState() && getProcessor().isRecording())
                {
                    getProcessor().setRecording(false);
                    spinlockedampsadd.printStatistics();
                    spinlockedampsget.printStatistics();
                    drawampsanno.printStatistics();
                    drawtimeanno.printStatistics();
                    drawamps.printStatistics();
                }
            }
        }
    private:

        std::unique_ptr<WaveDisplayComponent> waveDisplay;
        Label thresholdLabel{ {}, "Threshold level:" },
            releaseLabel{ {}, "Release Time:" };

        Slider thresholdSlider, releaseSlider, vscaleSlider, hscaleSlider;
        AudioProcessorValueTreeState::SliderAttachment thresholdAttachment, releaseAttachment, vscaleAttachment, hscaleAttachment;
        Colour backgroundColour;
        ScrollBar hscrollbar{ false };
        ShapeButton recordbutton{ "Rec",Colours::darkred, Colours::darkred, Colours::darkred  };
        Range<double> visibleRange;

        // these are used to persist the UI's size - the values are stored along with the
        // filter's other parameters, and the UI component will update them when it gets
        // resized.
        Value lastUIWidth, lastUIHeight;

        //==============================================================================
        JuceDemoPluginAudioProcessor& getProcessor() const
        {
            return static_cast<JuceDemoPluginAudioProcessor&> (processor);
        }

        // called when the stored window size changes
        void valueChanged (Value&) override
        {
            setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
        }
    };

    //==============================================================================
    template <typename FloatType>
    void process (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages)
    {
        auto thresholdParamValue  = state.getParameter ("threshold")->getNormalisableRange().convertFrom0to1(state.getParameter("threshold")->getValue());
        auto releaseParamValue = state.getParameter("release")->getNormalisableRange().convertFrom0to1(state.getParameter("release")->getValue());
        auto numSamples = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();
        if (buffer.hasBeenCleared()) return;
        captureBufferFloat.clear();
        for (int i = 0; i < numChannels; i++)
        {
            captureBufferFloat.copyFrom(i, 0, (float *)buffer.getReadPointer(i), numSamples);
        }

        // In case we have more outputs than inputs, we'll clear any output
        // channels that didn't contain input data, (because these aren't
        // guaranteed to be empty - they may contain garbage).
        for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
            buffer.clear (i, 0, numSamples);

        // and now get our synth to process these midi events and generate its output.
        // synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

        // Apply our delay effect to the new output..
        applyCompression (buffer, thresholdParamValue, releaseParamValue);


        for (int i = 0; i < numChannels; i++)
        {
            captureBufferFloat.copyFrom(numChannels + i, 0, (float *)buffer.getReadPointer(i), numSamples);
        }
        captureBufferFloat.setSize(4, numSamples, true);
        amps.add(captureBufferFloat);
    }

    template <typename FloatType>
    void applyCompression (AudioBuffer<FloatType>& buffer, float thresholdLevel, float releaseTime)
    {
        auto numSamples = buffer.getNumSamples();

        auto delayPos = 0;

        for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
        {
            auto channelData = buffer.getWritePointer (channel);
            
            for (auto i = 0; i < numSamples; ++i)
            {
                auto in = channelData[i];

            }
        }
    }
    juce::SpinLock mutex;

    bool _recording = false;

    AudioBuffer<float> captureBufferFloat;
    AudioBuffer<double> captureBufferDouble;

    CriticalSection trackPropertiesLock;
    TrackProperties trackProperties;

    static BusesProperties getBusesProperties()
    {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                .withOutput ("Output", AudioChannelSet::stereo(), true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceDemoPluginAudioProcessor)
};
