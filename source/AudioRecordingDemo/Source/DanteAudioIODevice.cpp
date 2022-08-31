#include "DanteAudioIODevice.h"
#include "juce_audio_devices/juce_audio_devices.h"   
#include <functional>

#define APP_NAME "AudioRecordingDemo"
#define APP_MODEL_NAME "Audio Recording Demo"
const Audinate::DAL::Id64 APP_MODEL_ID('A', 'U', 'R', 'C', 'R', 'D', 'D', 'M');

#define DEFAULT_BITS_PER_SAMPLE 16
/*
 *  Dante Application Library Access Token - Generated for bgillocksbc@yahoo.com
 *  Token issue date :  2022-08-15
 *  Token expiry date :  2022-09-14
 */
const unsigned char access_token[] =
"84b5272032a1ed08c8f973c0876c268c82a842f6b9e0125829645d7c75742aee87f771ec05428f5b" \
"fc8745186685375be166e668d8bfc93c243ee4b48016aa0035393738333300004175645f44414c00" \
"0000000000000000000000000000000000000000000000000000000000000000f0d3f96200000000" \
"f06021630000000054498adc59764064b2dadd5033c5930a0100000000000000";
static bool bufferAllocated = false;
static bool bufferReady = false;
static float** inputBuffers;
static float** outputBuffers;

static void myTransfer(const Audinate::DAL::AudioProperties& properties,
    const Audinate::DAL::AudioTransferParameters& params,
    unsigned int numChannels, unsigned int latencySamples)
{
    if (!bufferAllocated) return;

    unsigned int positionSamples =
        (params.mAvailableDataOffsetInPeriods * properties.mSamplesPerPeriod)
        % properties.mSamplesPerBuffer;
    unsigned int numSamples = params.mNumPeriodsAvailable * properties.mSamplesPerPeriod;

    uint16_t bitsPerSample = properties.mBytesPerSample * 8;
    size_t numCopyChannels = numChannels;
    uint8_t bytesPerSample = bitsPerSample / 8;

    // Copy non-interleaved u32 channel data to interleaved little endian audio data with
    // the configure bits per sample.
    uint32_t samplesLeft = numSamples;
    int i = 0;
    while (i < numSamples)
    {
        for (size_t chan = 0; chan < numCopyChannels; chan++)
        {
            const uint32_t* bufferPtr = reinterpret_cast<const uint32_t*>(properties.mRxChannelBuffers[chan] +
                (positionSamples % properties.mSamplesPerBuffer) * 4);
            inputBuffers[chan][i] = (float)*bufferPtr;
        }
        positionSamples++;
    }

    //waveWriter->putU32Samples(properties.mRxChannelBuffers, positionSamples,
    //	properties.mSamplesPerBuffer, numSamples);
    bufferReady = true;
    return;
}

DanteAudioIODeviceType::DanteAudioIODeviceType(Component* component) : AudioIODeviceType("Dante"), mComponent(component), mDeviceNames()
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
    mConfig.setNumRxChannels(2);
    mConfig.setRxChannelName(0, "Left");
    mConfig.setRxChannelName(1, "Right");
    mConfig.setNumTxChannels(2);
    mConfig.setTxChannelName(0, "Left");
    mConfig.setTxChannelName(1, "Right");
    mDalAppBase = new DAL::DalAppBase(APP_NAME, APP_MODEL_NAME, APP_MODEL_ID);
    mDalAppBase->setTransferFn(&myTransfer);
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
    mDeviceNames.clear();

    String rName = APP_NAME;
    rName.append("[2]", 20);
    mDeviceNames.add(rName);

    hasScanned = true;
};

StringArray DanteAudioIODeviceType::getDeviceNames(bool) const 
{
    if (!hasScanned) return StringArray();
    return mDeviceNames;
};
int DanteAudioIODeviceType::getDefaultDeviceIndex(bool) const 
{ 
    if (!hasScanned) return 0;
    return 0; 
};
int DanteAudioIODeviceType::getIndexOfDevice(AudioIODevice* d, bool) const 
{ 
    if (!hasScanned) return 0;
    return 0; 
};
bool DanteAudioIODeviceType::hasSeparateInputsAndOutputs() const { return false; };

AudioIODevice* DanteAudioIODeviceType::createDevice(const String& outputDeviceName,
    const String& inputDeviceName)
{
    if (!hasScanned) return nullptr; // need to call scanForDevices() before doing this
    if ((inputDeviceName != outputDeviceName) || outputDeviceName.isEmpty() || inputDeviceName.isEmpty()) return nullptr;

    std::unique_ptr<DanteAudioIODevice> device;

    device.reset(new DanteAudioIODevice(outputDeviceName,mDalAppBase));
    
    return device.release();
};
void DanteAudioIODeviceType::onAvailableChannelsChanged(std::vector<unsigned int> txChannelIds, std::vector<unsigned int> rxChannelIds)
{
    /*
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
    */
    mComponent->postCommandMessage(2);
}

DanteAudioIODevice::DanteAudioIODevice(const String& deviceName, DAL::DalAppBase* dalAppBase) : AudioIODevice(deviceName,"Dante"), Thread("JUCE DANTE"), mDalAppBase(dalAppBase){};

StringArray DanteAudioIODevice::getOutputChannelNames()
{
    StringArray outChannels;
    Audinate::DAL::InstanceConfig iConfig = mDalAppBase->getConfig();

    for (int i = 0; i < iConfig.getNumTxChannels(); ++i)
            outChannels.add(iConfig.getTxChannelName(i));

    return outChannels;
}

StringArray DanteAudioIODevice::getInputChannelNames()
{
    StringArray inChannels;

    Audinate::DAL::InstanceConfig iConfig = mDalAppBase->getConfig();

    for (int i = 0; i < iConfig.getNumRxChannels(); ++i)
        inChannels.add(iConfig.getRxChannelName(i));

    return inChannels;
}

Array<double> DanteAudioIODevice::getAvailableSampleRates() { return { 48000.0 }; };
Array<int> DanteAudioIODevice::getAvailableBufferSizes() { return { 960 }; };
int DanteAudioIODevice::getDefaultBufferSize() { return 960; };
String DanteAudioIODevice::open(const BigInteger& inputChannels,
    const BigInteger& outputChannels,
    double sampleRate,
    int bufferSizeSamples) 
{
    Audinate::DAL::AudioProperties properties;
    mDalAppBase->getAudioProperties(properties);
    mInputChannels = 0;
    for (int i = 0; i < properties.mRxActivatedChannelCount; ++i)
    {
        mInputChannels.setBit(i);
    }
    mOutputChannels = 0;
    for (int i = 0; i < properties.mTxActivatedChannelCount; ++i)
    {
        mOutputChannels.setBit(i);
    }

    //mInputChannels = inputChannels;
    //mOutputChannels = outputChannels;
    mSampleRate = sampleRate;

    //mDalAppBase->setTransferFn(std::bind(&DanteAudioIODevice::transfer,this, 
    //    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    startThread(8);
    Thread::sleep(5);
    /*
    if (inputDevice != nullptr && inputDevice->client != nullptr)
    {
        latencyIn = (int)(inputDevice->latencySamples + currentBufferSizeSamples);

        if (!inputDevice->start(currentBufferSizeSamples))
        {
            close();
            lastError = TRANS("Couldn't start the input device!");
            return lastError;
        }
    }

    if (outputDevice != nullptr && outputDevice->client != nullptr)
    {
        latencyOut = (int)(outputDevice->latencySamples + currentBufferSizeSamples);

        if (!outputDevice->start())
        {
            close();
            lastError = TRANS("Couldn't start the output device!");
            return lastError;
        }
    }*/
    isOpen_ = true;
    return "";
};
void DanteAudioIODevice::close() 
{
    stop();
    signalThreadShouldExit();

  //  if (inputDevice != nullptr)   SetEvent(inputDevice->clientEvent);
  //  if (outputDevice != nullptr)  SetEvent(outputDevice->clientEvent);

    stopThread(5000);

  //  if (inputDevice != nullptr)   inputDevice->close();
  //  if (outputDevice != nullptr)  outputDevice->close();

    isOpen_ = false;
};
bool DanteAudioIODevice::isOpen() { return isOpen_ && isThreadRunning(); }
bool DanteAudioIODevice::isPlaying()  { return isStarted && isOpen_ && isThreadRunning(); }
void DanteAudioIODevice::start(AudioIODeviceCallback* call) 
{ 
    if (isOpen_ && call != nullptr && !isStarted)
    {
        if (!isThreadRunning())
        {
            // something's gone wrong and the thread's stopped..
            isOpen_ = false;
            return;
        }

        call->audioDeviceAboutToStart(this);

        const ScopedLock sl(startStopLock);
        callback = call;
        isStarted = true;
    }
};
void DanteAudioIODevice::stop() 
{
    if (isStarted)
    {
        auto* callbackLocal = callback;

        {
            const ScopedLock sl(startStopLock);
            isStarted = false;
        }

        if (callbackLocal != nullptr)
            callbackLocal->audioDeviceStopped();
    }
};

void DanteAudioIODevice::run()
{
    // setMMThreadPriority();
    Audinate::DAL::AudioProperties properties;
    mDalAppBase->getAudioProperties(properties);
    mBufferSizeSamples = properties.mSamplesPerBuffer;
    inputBuffers = new float* [properties.mRxActivatedChannelCount];
    for (int i = 0; i < properties.mRxActivatedChannelCount; i++)
    {
        inputBuffers[i] = new float[mBufferSizeSamples + 32];
    }
    outputBuffers = new float* [properties.mTxActivatedChannelCount];
    for (int i = 0; i < properties.mTxActivatedChannelCount; i++)
    {
        outputBuffers[i] = new float[mBufferSizeSamples + 32];
    }
    //bufferAllocated = true;

    while (!threadShouldExit())
    {
        /*
        if ((outputDevice != nullptr && outputDevice->shouldShutdown)
            || (inputDevice != nullptr && inputDevice->shouldShutdown))
        {
            shouldShutdown = true;
      //      triggerAsyncUpdate();

            break;
        }
        */

        auto inputDeviceActive = (inputDevice != nullptr && inputDevice->isDeviceActivated());
        auto outputDeviceActive = (outputDevice != nullptr && outputDevice->isDeviceActivated());

     //   if (!inputDeviceActive && !outputDeviceActive)
     //       continue;

        if (inputDeviceActive)
        {
            //inputDevice->copyBuffersFromReservoir(inputBuffers, numInputBuffers, bufferSize);
        }

        {
            const ScopedTryLock sl(startStopLock);

            if (sl.isLocked() && isStarted && bufferReady)
                callback->audioDeviceIOCallbackWithContext(const_cast<const float**> (inputBuffers),
                    properties.mRxActivatedChannelCount,
                    outputBuffers,
                    properties.mTxActivatedChannelCount,
                    mBufferSizeSamples,
                    {});
 
        }
    }
}

String DanteAudioIODevice::getLastError() { return ""; };
int DanteAudioIODevice::getCurrentBufferSizeSamples() { return mBufferSizeSamples; };
double DanteAudioIODevice::getCurrentSampleRate() { return mSampleRate; };
int DanteAudioIODevice::getCurrentBitDepth() { return 0; };
BigInteger DanteAudioIODevice::getActiveOutputChannels() const { return mOutputChannels; };
BigInteger DanteAudioIODevice::getActiveInputChannels() const { return mInputChannels; };
int DanteAudioIODevice::getOutputLatencyInSamples() { return 0; };
int DanteAudioIODevice::getInputLatencyInSamples() { return 0; };
bool DanteAudioIODevice::setAudioPreprocessingEnabled(bool shouldBeEnabled) {
    return false;
};
int DanteAudioIODevice::getXRunCount() const noexcept {
    return 0;
};

