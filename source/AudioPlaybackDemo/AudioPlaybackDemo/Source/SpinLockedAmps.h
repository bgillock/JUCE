#include <JuceHeader.h>
namespace juce
{
    class SpinLockedAmps
    {
    public:
        void init(const int size, const bool isUsingDoublePrecision);
        template <typename FloatType>
        void add(AudioBuffer<FloatType>& newAmps);
        AudioBuffer<float> getFloat();
        AudioBuffer<double> getDouble();
        int getSize();
    };
}