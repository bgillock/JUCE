#include <JuceHeader.h>
namespace juce
{
    class SpinLockedAmps
    {
    public:
        void init(const int size, const bool isUsingDoublePrecision)
        {
            if (isUsingDoublePrecision)
            {
                _doubleAmps.setSize(2, size);
                _floatAmps.setSize(1, 1);
            }
            else
            {
                _floatAmps.setSize(2, size);
                _doubleAmps.setSize(1, 1);
            }
            _nSamples = 0;
            _startIndex = 0;
            _isFloat = !isUsingDoublePrecision;
        }

        template <typename FloatType>
        void add(AudioBuffer<FloatType>& newAmps)
        {
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
        }
        AudioBuffer<float> getFloat() noexcept
        {
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
            _startIndex = 0;
            _nSamples = 0;
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

            _startIndex = 0;
            _nSamples = 0;
            return _returnAmps;
        }
        int getSize() {
            return _nSamples;
        }
    private:
        juce::SpinLock mutex;
        bool _isFloat;
        int _nSamples;
        int _startIndex;
        int _maxNSamples = 0;
        AudioBuffer<float> _floatAmps;
        AudioBuffer<double> _doubleAmps;
    };
}