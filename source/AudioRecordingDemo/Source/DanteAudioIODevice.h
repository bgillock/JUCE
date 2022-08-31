#include <JuceHeader.h>
#include "DalAppBase.hpp"
class DanteAudioIODevice : public AudioIODevice, public Thread {

    String open(const BigInteger&, const BigInteger&, double, int) override;
    void close() override;

    void start(AudioIODeviceCallback*) override;
    void stop() override;

    Array<double> getAvailableSampleRates() override;
    Array<int> getAvailableBufferSizes() override;

    bool setAudioPreprocessingEnabled(bool) override;

    //==============================================================================
    bool isPlaying() override;
    bool isOpen() override;
    String getLastError() override;

    //==============================================================================
    StringArray getOutputChannelNames() override;
    StringArray getInputChannelNames() override;

    int getDefaultBufferSize() override;
    int getCurrentBufferSizeSamples() override;

    double getCurrentSampleRate() override;

    int getCurrentBitDepth() override;

    BigInteger getActiveOutputChannels() const override;
    BigInteger getActiveInputChannels() const override;

    int getOutputLatencyInSamples() override;
    int getInputLatencyInSamples() override;

    int getXRunCount() const noexcept override;
    void run() override;
public:
    DanteAudioIODevice(const String& deviceName, DAL::DalAppBase* dalAppBase);
private:
    //static void transfer(const Audinate::DAL::AudioProperties& properties,
    //    const Audinate::DAL::AudioTransferParameters& params,
    //    unsigned int numChannels, unsigned int latencySamples);  
    int actualNumChannels = 0;
    BigInteger mInputChannels;
    BigInteger mOutputChannels;
    double mSampleRate;
    int mBufferSizeSamples;
    DAL::DalAppBase* mDalAppBase;
    DAL::DalAppBase* inputDevice = nullptr;
    DAL::DalAppBase* outputDevice = nullptr;
    bool isOpen_ = false, isStarted = false;
    int currentBufferSizeSamples = 0;
    double currentSampleRate = 0;
    std::atomic<bool> shouldShutdown{ false }, deviceSampleRateChanged{ false };
    AudioIODeviceCallback* callback = {};
    CriticalSection startStopLock;
};

class DanteAudioIODeviceType : public AudioIODeviceType {
public:
    DanteAudioIODeviceType(Component*);
    virtual void scanForDevices() override;
    virtual StringArray getDeviceNames(bool) const override;
    virtual int getDefaultDeviceIndex(bool) const override;
    virtual int getIndexOfDevice(AudioIODevice* d, bool) const override;
    virtual bool hasSeparateInputsAndOutputs() const override;
    virtual AudioIODevice* createDevice(const String& outputDeviceName,
        const String& inputDeviceName) override;        
    
private:
    bool mChannelsReady = false;
    DAL::DalAppBase* mDalAppBase = nullptr;
    DAL::DalConfig mConfig;
    std::shared_ptr<Audinate::DAL::Connections> mConnections;  
    StringArray mDeviceNames;
    Component* mComponent;
    bool hasScanned = false;
    void onAvailableChannelsChanged(std::vector<unsigned int> txChannelIds, std::vector<unsigned int> rxChannelIds);
};
