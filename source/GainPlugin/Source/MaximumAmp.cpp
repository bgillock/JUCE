/*
  ==============================================================================

    MeterBuffer.cpp
    Created: 24 Nov 2022 9:51:30am
    Author:  bgill

  ==============================================================================
*/
#include <JuceHeader.h>
#include "MaximumAmp.h"

MaximumAmp::MaximumAmp(double min, double max, int nLevels, int peakHoldTimes) 
{ 
    _nLevels = nLevels;
    _minAmp = min;
    _maxAmp = max;
    _peakHoldTimes = peakHoldTimes;
    _levels = new float[_nLevels];
    _signal = false;
    _clipped = false;
    _peakAmp = -144.0; 
};
void MaximumAmp::setNLevels(int n)
{
    delete _levels;
    _nLevels = n;
    _levels = new float[_nLevels];
}
void MaximumAmp::capture(AudioBuffer<float> amps, int channel)
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        float db = Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
        if (db > _peakAmp) _peakAmp = db;
        _clipped = _clipped || (db > _maxAmp);
        _signal = _signal || (amps.getMagnitude(channel, 0, amps.getNumSamples()) > 0.0);
    }
}
void MaximumAmp::capture(AudioBuffer<double> amps, int channel)
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        float db = Decibels::gainToDecibels(amps.getRMSLevel(channel, 0, amps.getNumSamples()));
        if (db > _peakAmp) _peakAmp = db;
        _clipped = _clipped || (db > _maxAmp);
        _signal = _signal || (amps.getMagnitude(channel, 0, amps.getNumSamples()) > 0.0);
    }
}
float* MaximumAmp::getLevels()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (++_peakTimes > _peakHoldTimes)
    {
        _peakTimes = 0;
        _peakhold = -144.0;
    }

    if (_peakAmp > _peakhold)
    {
        _peakhold = _peakAmp;
        _peakTimes = 0;
    }
    
    int l = (int)((float)(_peakAmp - _minAmp) / (_maxAmp - _minAmp) * (float)_nLevels);
    int h = (int)((float)(_peakhold - _minAmp) / (_maxAmp - _minAmp) * (float)_nLevels);
    for (int i = 0; i < _nLevels; i++) {
        _levels[i] = 0.0;
        if (i == h) _levels[i] = 1.0;
        if (i <= l) _levels[i] = 1.0;
    }
    return _levels;
}
bool MaximumAmp::clipped()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    return _clipped;
}
void MaximumAmp::setClipped(bool clip)
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        _clipped = false;
    }
}
bool MaximumAmp::signal()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    return _signal;
}
void MaximumAmp::clear()
{
    const juce::SpinLock::ScopedTryLockType lock(mutex);
    if (lock.isLocked())
    {
        _peakAmp = -144.0;
        _signal = false;
    }
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
    int get_NBuffs()
    {
        return _nBuffs;
    }
    int getNBins()
    {
        return _nBins;
    }
    int getTimeWeightedHistogram(float *histOut)
    {
        // Return 2d array of histogram values starting with the oldest in time and maxBin.
        int b = (_currentBuff - 1); if (b < 0) b = _nBuffs - 1;
        int n = 0;
        while (n < _nBuffs)
        {
            for (int h = 0; h < _nBins; h++)
            {

            }
            b--; if (b < 0) b = _nBuffs - 1;
            n++;
        }
        _currentBuff++;
        if (_currentBuff >= _nBuffs) _currentBuff = 0;
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