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
   
    auto la = _leftAnnoWidth > 1.0 ? r.removeFromLeft(_leftAnnoWidth) : r.removeFromLeft((float)r.getWidth() * _leftAnnoWidth);
    auto ra = _rightAnnoWidth > 1.0 ? r.removeFromRight(_rightAnnoWidth) : r.removeFromRight((float)r.getWidth() * _rightAnnoWidth);

    leftLevelMeter.setBounds(r.removeFromLeft(r.getWidth() / 2));
    rightLevelMeter.setBounds(r);
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

void StereoLevelMeter::capture(AudioBuffer<float> amps)
{
    leftLevelMeter.capture(amps, 0);
    rightLevelMeter.capture(amps, 1);
}
void StereoLevelMeter::capture(AudioBuffer<double> amps)
{
    leftLevelMeter.capture(amps, 0);
    rightLevelMeter.capture(amps, 1);
}

UADLevelMeter::UADLevelMeter(int marginTop, int marginBottom ) :
    _mTop(marginTop),
    _mBottom(marginBottom),
    maxAmp(-54.0, 0.0, 20, 10) 
{
    _lightImages = ImageFileFormat::loadFrom(BinaryData::APILights_png, BinaryData::APILights_pngSize);
};

void UADLevelMeter::resized()
{
    auto area = getBounds();
    int topy = _mTop;
    int bottomy = area.getHeight() - _mBottom;

    _nLights = (int)((float)(bottomy - topy + 1) / (_lightheight + _spacing));
    maxAmp.setNLevels(_nLights);

    if (_lightImageIndexes != nullptr) delete _lightImageIndexes;
    _lightImageIndexes = new int[_nLights];
    int orangelight = (int)((float)_nLights * 0.66f);
    for (int l = 0; l < _nLights; l++)
    {
        int thisImage = 2; // default to off, on = +3
        if (l == _nLights - 1) thisImage = 0;
        else if (l >= orangelight) thisImage = 1;
        _lightImageIndexes[l] = thisImage;
    }
}

int UADLevelMeter::getActualHeight()
{
    return _mTop + (_nLights * (_lightheight + _spacing)) + _mBottom;
}
void UADLevelMeter::drawLight(Graphics& g, int x, int y, int width, int height, float *levels, int l)
{
    int index = _lightImageIndexes[l];
    if (levels[l] == 1.0) index+=3;

    g.drawImage(_lightImages, x, y, (int)_lightwidth, (int)_lightheight, 0, index * ((int)_lightheight + (int)_spacing), (int)_lightwidth, (int)_lightheight );
}

void UADLevelMeter::drawClipped(Graphics& g, int x, int y, int width, int height, bool clipped)
{
    int index = 0;
    if (clipped) index = 3;
    g.drawImage(_lightImages, x, y, (int)_lightwidth, (int)_lightheight, 0, index * ((int)_lightheight + (int)_spacing), (int)_lightwidth, (int)_lightheight);
}

void UADLevelMeter::drawSignal(Graphics& g, int x, int y, int width, int height, bool signal)
{
    int index = 2;
    if (signal) index = 5;
    g.drawImage(_lightImages, x, y, (int)_lightwidth, (int)_lightheight, 0, index * ((int)_lightheight + (int)_spacing), (int)_lightwidth, (int)_lightheight);
}

void UADLevelMeter::paint(Graphics& g)
{

    //g.setColour(Colours::red);
    //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);

    auto levels = maxAmp.getLevels();
    int nlights = maxAmp.getNLevels();

    int y = _mTop;
    int centerx = getBounds().getWidth() / 2;

    int tx = centerx - _lightwidth / 2;
    
    drawClipped(g, tx, (int)(_mTop - _spacing - _clippedheight), (int)_lightwidth, (int)_clippedheight, maxAmp.clipped());

    for (int l = _nLights - 1; l >= 0; l--)
    {
        drawLight(g, tx, y, _lightwidth, _lightheight, levels, l);
        y += _lightheight + _spacing;
    }
 
    drawSignal(g, tx, _mTop + (_nLights * (int)(_lightheight + _spacing)), (int)_lightwidth, (int)_signalheight, maxAmp.signal());

    maxAmp.clear();
    return;
};

void UADLevelMeter::capture(AudioBuffer<float> amps, int channel)
{
    maxAmp.capture(amps, channel);
}
void UADLevelMeter::capture(AudioBuffer<double> amps, int channel)
{
    maxAmp.capture(amps, channel);
}

DrawnLEDLevelMeter::DrawnLEDLevelMeter(int marginTop, int marginBottom) :
    _mTop(marginTop),
    _mBottom(marginBottom),
    maxAmp(-54.0, 0.0, 20, 10)
{

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
void DrawnLEDLevelMeter::drawLight(Graphics& g, int x, int y, int width, int height, float* levels, int l)
{
    g.setColour(Colours::black); // off color
    if (levels[l] == 1.0) g.setColour(_lightColors[l]);
    g.fillRect(x, y, (int)_lightwidth, (int)_lightheight);

    g.setColour(Colours::grey); // border color
    g.drawRect(x, y, (int)_lightwidth, (int)_lightheight, (int)_lightborder);
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

void DrawnLEDLevelMeter::paint(Graphics& g)
{

    //g.setColour(Colours::red);
    //g.drawRect(0, 0, getBounds().getWidth(), getBounds().getHeight(), 1.0);

    auto levels = maxAmp.getLevels();
    int nlights = maxAmp.getNLevels();

    int y = _mTop;
    int centerx = getBounds().getWidth() / 2;

    int tx = centerx - _lightwidth / 2;

    drawClipped(g, tx, (int)(_mTop - _spacing - _clippedheight), (int)_lightwidth, (int)_clippedheight, maxAmp.clipped());

    for (int l = _nLights - 1; l >= 0; l--)
    {
        drawLight(g, tx, y, _lightwidth, _lightheight, levels, l);
        y += _lightheight + _spacing;
    }

    drawSignal(g, tx, _mTop + (_nLights * (int)(_lightheight + _spacing)), (int)_lightwidth, (int)_signalheight, maxAmp.signal());

    maxAmp.clear();
    return;
};

void DrawnLEDLevelMeter::capture(AudioBuffer<float> amps, int channel)
{
    maxAmp.capture(amps, channel);
}
void DrawnLEDLevelMeter::capture(AudioBuffer<double> amps, int channel)
{
    maxAmp.capture(amps, channel);
}

