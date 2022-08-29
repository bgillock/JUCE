#include <JuceHeader.h>
#include "DalAppBase.hpp"
class DanteAudioIODevice : public AudioIODevice {

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

public:
    DanteAudioIODevice(const String& deviceName, DAL::DalAppBase* dalAppBase);
private:
    DanteAudioIODevice* inputDevice = nullptr;
    DanteAudioIODevice* outputDevice = nullptr;    
    int actualNumChannels = 0;
    BigInteger mInputChannels;
    BigInteger mOutputChannels;
    double mSampleRate;
    int mBufferSizeSamples;
    DAL::DalAppBase* mDalAppBase;
};

class DanteAudioIODeviceType : public AudioIODeviceType {
public:
    DanteAudioIODeviceType(std::shared_ptr<Component>);
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
    std::shared_ptr<Component> mComponent;
    bool hasScanned = false;
    void onAvailableChannelsChanged(std::vector<unsigned int> txChannelIds, std::vector<unsigned int> rxChannelIds);
};
