/*
  ==============================================================================

    StereoLevelMeter.cpp
    Created: 24 Nov 2022 9:45:47am
    Author:  bgill

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MaximumAmp.h"
#include "StereoLevelMeter.h"

//----------------------------------------------------------------------------------------------------------------------
StereoLevelMeter::StereoLevelMeter(int minAmp, int maxAmp, int incAmp, int marginTop, int marginBottom, float leftAnnoWidth, float rightAnnoWidth) :
    leftLevelMeter(marginTop, marginBottom),
    rightLevelMeter(marginTop, marginBottom),
    leftAnno(minAmp, maxAmp, incAmp, marginTop, marginBottom, leftAnnoWidth, Justification::left),
    rightAnno(minAmp, maxAmp, incAmp, marginTop, marginBottom, rightAnnoWidth, Justification::right)
{
    _leftAnnoWidth = leftAnnoWidth;
    _rightAnnoWidth = rightAnnoWidth;
    //startTimerHz(100);
    addAndMakeVisible(leftLevelMeter);
    addAndMakeVisible(rightLevelMeter);
    if (leftAnnoWidth > 0.0) addAndMakeVisible(leftAnno);
    if (rightAnnoWidth > 0.0) addAndMakeVisible(rightAnno);
};

void StereoLevelMeter::timerCallback() 
{
    repaint();
};

void StereoLevelMeter::resized()
{
    auto r = getLocalBounds();
   
    auto la = _leftAnnoWidth > 1.0 ? r.removeFromLeft((int)_leftAnnoWidth) : r.removeFromLeft((int)(r.getWidth() * _leftAnnoWidth));
    auto ra = _rightAnnoWidth > 1.0 ? r.removeFromRight((int)_rightAnnoWidth) : r.removeFromRight((int)(r.getWidth() * _rightAnnoWidth));

    leftLevelMeter.setBounds(r.removeFromLeft(r.getWidth() / 2));
    rightLevelMeter.setBounds(r); 
    
    leftLevelMeter.resized();
    rightLevelMeter.resized();
    
    if (_leftAnnoWidth > 0.0)
    {
        la.setHeight(leftLevelMeter.getActualHeight());
        leftAnno.setBounds(la);
    }
    if (_rightAnnoWidth > 0.0)
    {
        ra.setHeight(rightLevelMeter.getActualHeight());
        rightAnno.setBounds(ra);
    }
};

int StereoLevelMeter::getActualHeight()
{
    return jmax(leftLevelMeter.getActualHeight(), rightLevelMeter.getActualHeight());
}

void StereoLevelMeter::clearClipped()
{
    leftLevelMeter.clearClipped();
    rightLevelMeter.clearClipped();
};
void StereoLevelMeter::setRange(Range<double> r)
{
    if (leftLevelMeter.canSetRange()) 
    {
        leftLevelMeter.setOrangeLevel(r.getStart());
        leftLevelMeter.setRedLevel(r.getEnd());
    }
    if (rightLevelMeter.canSetRange())
    {
        rightLevelMeter.setOrangeLevel(r.getStart());
        rightLevelMeter.setRedLevel(r.getEnd());
    }
}
void StereoLevelMeter::capture(AudioBuffer<float> amps)
{
    leftLevelMeter.capture(amps, 0);
    rightLevelMeter.capture(amps, 1);
};
void StereoLevelMeter::capture(AudioBuffer<double> amps)
{
    leftLevelMeter.capture(amps, 0);
    rightLevelMeter.capture(amps, 1);
};

//-----------------------------------------------------------------------------------------------------------
void LevelMeter::paint(Graphics& g)
{

    //g.setColour(Colours::red);
    //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);

    auto levels = maxAmp.getLevels();
    _nLights = maxAmp.getNLevels();

    int y = _mTop;
    int centerx = getBounds().getWidth() / 2;

    int tx = centerx - _lightwidth / 2;

    drawClipped(g, tx, _mTop - _spacing - _clippedheight, _lightwidth, _clippedheight, maxAmp.clipped());

    for (int l = _nLights - 1; l >= 0; l--)
    {
        drawLight(g, tx, y, _lightwidth, _lightheight, levels, l);
        y += _lightheight + _spacing;
    }

    drawSignal(g, tx, _mTop + (_nLights * (_lightheight + _spacing)), _lightwidth, _signalheight, maxAmp.signal());

    maxAmp.clear();
    return;
};

void LevelMeter::capture(AudioBuffer<float> amps, int channel)
{
    maxAmp.capture(amps, channel);
}
void LevelMeter::capture(AudioBuffer<double> amps, int channel)
{
    maxAmp.capture(amps, channel);
}

//---------------------------------------------------------------------------------------------------------------------
UADLevelMeter::UADLevelMeter(int marginTop, int marginBottom) :
    LevelMeter(marginTop,marginBottom)
{
    _orangeLevel = -18.0;
    _redLevel = -3.0;
    _peakholdTimes = 10; // number of times to leave peak 
    _lightheight = 16;
    _lightwidth = 16;
    _spacing = 2;
    _clippedheight = 16;
    _signalheight = 16;
    _lightImages = ImageFileFormat::loadFrom(BinaryData::APILights_png, BinaryData::APILights_pngSize);
};

void UADLevelMeter::resized()
{
    auto area = getBounds();
    int topy = _mTop;
    int bottomy = area.getHeight() - _mBottom;

    _nLights = (int)((float)(bottomy - topy + 1) / (_lightheight + _spacing));
    maxAmp.setNLevels(_nLights);
    auto mindb = maxAmp.getMinAmp();
    auto maxdb = maxAmp.getMaxAmp();
    float dbPerLight = ((maxdb - mindb) / (float)_nLights);

    if (_lightImageIndexes != nullptr) delete _lightImageIndexes;
    _lightImageIndexes = new int[_nLights];

    for (int l = 0; l < _nLights; l++)
    {
        int thisImage = 2; // default to off, on = +3
        float thisdb = mindb + ((float)l * dbPerLight);
        if (thisdb > _redLevel) thisImage = 0;
        else if (thisdb > _orangeLevel) thisImage = 1;
        _lightImageIndexes[l] = thisImage;
    }
}

int UADLevelMeter::getActualHeight()
{
    return _mTop + (_nLights * (_lightheight + _spacing)) + _mBottom;
}

void UADLevelMeter::setRedLevel(float level)
{
    _redLevel = level;
}
void UADLevelMeter::setOrangeLevel(float level)
{
    _orangeLevel = level;
}
void UADLevelMeter::clearClipped()
{
    maxAmp.setClipped(false);
}
void UADLevelMeter::drawLight(Graphics& g, int x, int y, int width, int height, float *levels, int l)
{
    int index = _lightImageIndexes[l];
    if (levels[l] == 1.0) index+=3;

    g.drawImage(_lightImages, x, y, width, height, 0, index * (height + _spacing), width, height );
}

void UADLevelMeter::drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped)
{
    int index = 0;
    if (clipped) index = 3;
    g.drawImage(_lightImages, x, y, width, height, 0, index * (height + _spacing), width, height);
}

void UADLevelMeter::drawSignal(Graphics& g, int x, int y, int width, int height, bool signal)
{
    int index = 2;
    if (signal) index = 5;
    g.drawImage(_lightImages, x, y, width, height, 0, index * (height + _spacing), width, height);
}

//---------------------------------------------------------------------------------------------------
DrawnLEDLevelMeter::DrawnLEDLevelMeter(int marginTop, int marginBottom) :
    LevelMeter(marginTop,marginBottom)
{
    _peakholdTimes = 10; // number of times to leave peak 
    _lightheight = 9;
    _lightwidth = 17;
    _spacing = 1;
    _clippedheight = 10;
    _signalheight = 10;
    _nLights = 10;
};

void DrawnLEDLevelMeter::resized()
{
    auto area = getBounds();
    int topy = _mTop;
    int bottomy = area.getHeight() - _mBottom;

    _nLights = (int)((float)(bottomy - topy + 1) / (_lightheight + _spacing));
    maxAmp.setNLevels(_nLights);

    if (_lightColors != nullptr) delete _lightColors;
    _lightColors = new Colour[_nLights];
    int orangelight = (int)((float)_nLights * 0.66f);
    for (int l = 0; l < _nLights; l++)
    {
        Colour thiscolor = Colour::fromRGB(0, 0, 0);
        if (l == _nLights - 1) thiscolor = Colour::fromRGB(255, 0, 0);
        else if (l >= orangelight) thiscolor = Colours::orange;
        else thiscolor = Colour::fromRGB(0, 255, 0);
        _lightColors[l] = thiscolor;
    }
}

int DrawnLEDLevelMeter::getActualHeight()
{
    return _mTop + (_nLights * (_lightheight + _spacing)) + _mBottom;
}

void DrawnLEDLevelMeter::clearClipped()
{
    maxAmp.setClipped(false);
}

void DrawnLEDLevelMeter::drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l)
{
    g.setColour(Colours::black); // off color
    if (levels[l] == 1.0) g.setColour(_lightColors[l]);
    g.fillRect(x, y, width, height);

    g.setColour(Colours::grey); // border color
    g.drawRect(x, y, width, height, (int)_lightborder);
}

void DrawnLEDLevelMeter::drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped)
{
    g.setColour(Colours::black); // off color
    if (clipped) g.setColour(Colours::red);
    g.fillRect(x, y, width, height);

    g.setColour(Colours::grey); // border color
    g.drawRect(x, y, width, height, (int)_lightborder);
}

void DrawnLEDLevelMeter::drawSignal(Graphics& g, int x, int y, int width, int height, bool signal)
{
    g.setColour(Colours::black); // off color
    if (signal) g.setColour(Colour::fromRGB(0, 255, 0));
    g.fillRect(x, y, width, height);

    g.setColour(Colours::grey); // border color
    g.drawRect(x, y, width, height, (int)_lightborder);
}

class VUHistogram
{
public:
    VUHistogram(int nBins, int nBuffs, double minBin, double maxBin)
    {
        _nBins = nBins;
        _nBuffs = nBuffs;
        _minBin = minBin;
        _maxBin = maxBin;
        _hist = new int* [_nBuffs];
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