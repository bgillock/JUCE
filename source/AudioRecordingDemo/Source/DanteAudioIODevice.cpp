#include "DanteAudioIODevice.h"
#include "juce_audio_devices/juce_audio_devices.h"   

#define APP_NAME "AudioRecordingDemo"
#define APP_MODEL_NAME "Audio Recording Demo"
const Audinate::DAL::Id64 APP_MODEL_ID('A', 'U', 'R', 'C', 'R', 'D', 'D', 'M');

#define DEFAULT_BITS_PER_SAMPLE 16
const unsigned char access_token[] =
"c37078d1e3ea18fd3ddedc4fcf7e75be734f156fe2233bc26a9a4e4ac148dd06110b467b7150b3e5" \
"fc1f243b3174ac08d80860111719b27faa9bc5ee2ce1240e35393738333300004175645f44414c00" \
"00000000000000000000000000000000000000000000000000000000000000009d25e76200000000" \
"9db20e6300000000451bb4cc4f164ffa941324f9d3e6b8540100000000000000";

DanteAudioIODeviceType::DanteAudioIODeviceType(std::shared_ptr<Component> component) : AudioIODeviceType("Dante"), mComponent(component), mOutputDeviceNames(), mInputDeviceNames(), mOutputDevices(), mInputDevices()
{
    mConfig.setInterfaceIndex(11);
    mConfig.setTimeSource(Audinate::DAL::TimeSource::RxAudio);
    mConfig.setManufacturerName("BitRate27");
    Audinate::DAL::Version dalAppVersion(1, 0, 0);
    mConfig.setManufacturerVersion(dalAppVersion);
    mConfig.setDefaultName(APP_NAME);
    mConfig.setModelName(APP_MODEL_NAME);
    mConfig.setModelId(APP_MODEL_ID);
    mConfig.setProcessPath("D:\\Audio\\Repos\\Audinate\\bin");
    mConfig.setLoggingPath("D:\\Audio\\Repos\\Audinate\\logs");

    mDalAppBase = new DAL::DalAppBase(APP_NAME, APP_MODEL_NAME, APP_MODEL_ID);
    mDalAppBase->init(access_token, mConfig, true);
    mDalAppBase->run();

    mComponent->postCommandMessage(0);

    // Create ConnectionsAPI
    Audinate::DAL::ConnectionsConfig config;

    config.setArcpSocketDescriptor(mDalAppBase->getConfig().getProtocolSocketDescriptor(Audinate::DAL::Protocol::Arcp));
    config.setConmonClientSocketDescriptor(mDalAppBase->getConfig().getProtocolSocketDescriptor(Audinate::DAL::Protocol::ConmonClient));
    config.setDomainClientProxySocketDescriptor(mDalAppBase->getConfig().getProtocolSocketDescriptor(Audinate::DAL::Protocol::DomainClientProxy));
#ifdef _WIN32
    config.setMdnsClientSocketDescriptor(mDalAppBase->getConfig().getProtocolSocketDescriptor(Audinate::DAL::Protocol::MdnsClient));
#endif

    mConnections = Audinate::DAL::createConnections(mDalAppBase->getDal(), config);
    mComponent->postCommandMessage(1); 
    // Setup a call back for when the Channels are ready
    mConnections->setAvailableChannelsChangedFn(bind(&DanteAudioIODeviceType::onAvailableChannelsChanged, this, std::placeholders::_1, std::placeholders::_2));
};
void DanteAudioIODeviceType::scanForDevices()
{
};

StringArray DanteAudioIODeviceType::getDeviceNames(bool reqInput) const 
{
    if (!mChannelsReady) return StringArray();
    return reqInput ? mInputDeviceNames : mOutputDeviceNames;
};
int DanteAudioIODeviceType::getDefaultDeviceIndex(bool) const 
{ 
    return 0; 
};
int DanteAudioIODeviceType::getIndexOfDevice(AudioIODevice* d, bool) const 
{ 
    return 0; 
};
bool DanteAudioIODeviceType::hasSeparateInputsAndOutputs() const { return true; };
AudioIODevice* DanteAudioIODeviceType::createDevice(const String& outputDeviceName,
    const String& inputDeviceName)
{
    return new DanteAudioIODevice(outputDeviceName);
};
void DanteAudioIODeviceType::onAvailableChannelsChanged(std::vector<unsigned int> txChannelIds, std::vector<unsigned int> rxChannelIds)
{
    mOutputDeviceNames.clear();
    for (auto txChannelId : txChannelIds)
    {
        auto available = mConnections->getAvailableDestinations(txChannelId);
        for (auto x : available.mDevices)
        {
            if (mOutputDeviceNames.indexOf(x.mDeviceName) == -1)
            {
                mOutputDeviceNames.add(x.mDeviceName);
                auto newDevice = createDevice(x.mDeviceName, "");
                mOutputDevices.add(newDevice);
            }
            for (auto y : x.mChannelNames)
            {

            }
        }
    }
    mInputDeviceNames.clear();
    for (auto rxChannelId : rxChannelIds)
    {
        auto available = mConnections->getAvailableDestinations(rxChannelId);
        for (auto x : available.mDevices)
        {
            if (mInputDeviceNames.indexOf(x.mDeviceName) == -1)
            {
                mInputDeviceNames.add(x.mDeviceName);
                mInputDevices.add(createDevice(x.mDeviceName,""));
            }
            for (auto y : x.mChannelNames)
            {

            }
        }
    }
    mChannelsReady = true;
    mComponent->postCommandMessage(2);
}

DanteAudioIODevice::DanteAudioIODevice(const String& deviceName) : AudioIODevice(deviceName,"Dante") {};

StringArray DanteAudioIODevice::getOutputChannelNames()
{
    StringArray outChannels;

    if (outputDevice != nullptr)
        for (int i = 1; i <= outputDevice->actualNumChannels; ++i)
            outChannels.add("Output channel " + String(i));

    return outChannels;
}

StringArray DanteAudioIODevice::getInputChannelNames()
{
    StringArray inChannels;

    if (inputDevice != nullptr)
        for (int i = 1; i <= inputDevice->actualNumChannels; ++i)
            inChannels.add("Input channel " + String(i));

    return inChannels;
}

Array<double> DanteAudioIODevice::getAvailableSampleRates() { return { 48000.0 }; };
Array<int> DanteAudioIODevice::getAvailableBufferSizes() { return { 960 }; };
int DanteAudioIODevice::getDefaultBufferSize() { return 0; };
String DanteAudioIODevice::open(const BigInteger& inputChannels,
    const BigInteger& outputChannels,
    double sampleRate,
    int bufferSizeSamples) 
{
    return "";
};
void DanteAudioIODevice::close() {};
bool DanteAudioIODevice::isOpen() { return 0; };
void DanteAudioIODevice::start(AudioIODeviceCallback* callback) { };
void DanteAudioIODevice::stop() { };
bool DanteAudioIODevice::isPlaying() { return 0; };
String DanteAudioIODevice::getLastError() { return ""; };
int DanteAudioIODevice::getCurrentBufferSizeSamples() { return 960; };
double DanteAudioIODevice::getCurrentSampleRate() { return 48000.0; };
int DanteAudioIODevice::getCurrentBitDepth() { return 0; };
BigInteger DanteAudioIODevice::getActiveOutputChannels() const { return 0; };
BigInteger DanteAudioIODevice::getActiveInputChannels() const { return 0; };
int DanteAudioIODevice::getOutputLatencyInSamples() { return 0; };
int DanteAudioIODevice::getInputLatencyInSamples() { return 0; };
bool DanteAudioIODevice::setAudioPreprocessingEnabled(bool shouldBeEnabled) {
    return false;
};
int DanteAudioIODevice::getXRunCount() const noexcept {
    return 0;
};

