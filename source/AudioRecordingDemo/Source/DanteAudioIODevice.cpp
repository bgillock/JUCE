#include "DanteAudioIODevice.h"
#include "juce_audio_devices/juce_audio_devices.h"   
#include <functional>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath> 
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
static int samplesInBuffers = 0;
static float** inputBuffers;
static float** outputBuffers;
static CriticalSection bufferLock;
static std::ofstream mt("myTransfer.txt", std::ios::out | std::ios::app);
static AudioIODeviceCallback* callback = nullptr;
static void convert24BitSignedtoFloat(const uint32_t* from, float* to) 
{
    unsigned char* f = (unsigned char*)from;
    int32_t it;
    unsigned char* t = (unsigned char*)&it;
    t[3] = 0x00;
    char tc = f[3] >> 7;
    if (tc == 0x01) t[3] = 0xff;
    t[0] = f[1];
    t[1] = f[2];
    t[2] = f[3];
    *to = (float)it / 8388607.0f;
}
static void printStatus(std::ofstream& mt, String sa, int a)
{
    mt << sa << "=" << String(a) << std::endl << std::flush;
}
static void myTransfer(const Audinate::DAL::AudioProperties& properties,
    const Audinate::DAL::AudioTransferParameters& params,
    unsigned int numChannels, unsigned int latencySamples)
{

    if (!bufferAllocated) return;
 //   const ScopedTryLock sl(bufferLock);
 //   if (sl.isLocked())
    try 
    {
        unsigned int positionSamples =
            (params.mAvailableDataOffsetInPeriods * properties.mSamplesPerPeriod)
            % properties.mSamplesPerBuffer;
        unsigned int numSamples = params.mNumPeriodsAvailable * properties.mSamplesPerPeriod;
      
        //printStatus(mt, "numSamples", numSamples);

        uint32_t samplesLeft = numSamples;
        int i = samplesInBuffers;

        while (i < numSamples)
        {
            for (size_t chan = 0; chan < numChannels; chan++)
            {
                const uint32_t* bufferPtr = reinterpret_cast<const uint32_t*>(properties.mRxChannelBuffers[chan] +
                    (positionSamples % properties.mSamplesPerBuffer) * 4);
                convert24BitSignedtoFloat(bufferPtr,&inputBuffers[chan][i]);
            }
            positionSamples++;
            i++;
        }
        samplesInBuffers += numSamples;
        if ((callback != nullptr))
        {
            //printStatus(mt, "mSamplesInBuffers", samplesInBuffers);
            callback->audioDeviceIOCallbackWithContext(const_cast<const float**> (inputBuffers),
                properties.mRxActivatedChannelCount,
                outputBuffers,
                properties.mTxActivatedChannelCount,
                samplesInBuffers,
                {});
            samplesInBuffers = 0;
        }
        
    }
    catch (const std::exception& exception)
    {
        bufferAllocated = false;
    }
    return;
}

DanteAudioIODeviceType::DanteAudioIODeviceType() : AudioIODeviceType("Dante"), mDeviceNames()
{
   
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

    device.reset(new DanteAudioIODevice(outputDeviceName));
    
    return device.release();
};

DanteAudioIODevice::DanteAudioIODevice(const String& deviceName) : AudioIODevice(deviceName,"Dante"), Thread("JUCE DANTE")
{   
    mConfig.setInterfaceName(L"Ethernet");
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
    inputDevice = new DAL::DalAppBase(APP_NAME, APP_MODEL_NAME, APP_MODEL_ID);
    inputDevice->setTransferFn(&myTransfer);
    inputDevice->init(access_token, mConfig, true);
    inputDevice->run();

};
DanteAudioIODevice::~DanteAudioIODevice() 
{
    mt.close();
    stop();
}
StringArray DanteAudioIODevice::getOutputChannelNames()
{
    StringArray outChannels;
    Audinate::DAL::InstanceConfig iConfig = inputDevice->getConfig();

    for (int i = 0; i < iConfig.getNumTxChannels(); ++i)
            outChannels.add(iConfig.getTxChannelName(i));

    return outChannels;
}

StringArray DanteAudioIODevice::getInputChannelNames()
{
    StringArray inChannels;

    Audinate::DAL::InstanceConfig iConfig = inputDevice->getConfig();

    for (int i = 0; i < iConfig.getNumRxChannels(); ++i)
        inChannels.add(iConfig.getRxChannelName(i));

    return inChannels;
}

Array<double> DanteAudioIODevice::getAvailableSampleRates() { return { 48000.0 }; };
Array<int> DanteAudioIODevice::getAvailableBufferSizes() { return { 128 }; };
int DanteAudioIODevice::getDefaultBufferSize() { return 128; };
String DanteAudioIODevice::open(const BigInteger& inputChannels,
    const BigInteger& outputChannels,
    double sampleRate,
    int bufferSizeSamples) 
{   

    mInputChannels = inputChannels;
    mOutputChannels = outputChannels;
    mSampleRate = sampleRate;
    mBufferSizeSamples = bufferSizeSamples;

    startThread(8);

    //inputDevice->setTransferFn(std::bind(&DanteAudioIODevice::transfer,this, 
    //    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));


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

        const ScopedLock sl(bufferLock);
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
            const ScopedLock sl(bufferLock);
            isStarted = false;
        }

        if (callbackLocal != nullptr)
            callbackLocal->audioDeviceStopped();
    }
};

void DanteAudioIODevice::run()
{
    // Wait for local device to become activated (Apec running)
    while (!inputDevice->isDeviceActivated())
    {
        Thread::sleep(1000);
    }
    Audinate::DAL::AudioProperties properties;
    inputDevice->getAudioProperties(properties);
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

    inputBuffers = new float* [properties.mRxActivatedChannelCount];
    for (int i = 0; i < properties.mRxActivatedChannelCount; i++)
    {
        inputBuffers[i] = new float[(mBufferSizeSamples * properties.mPeriodsPerBuffer) + 32];
    }
    outputBuffers = new float* [properties.mTxActivatedChannelCount];
    for (int i = 0; i < properties.mTxActivatedChannelCount; i++)
    {
        outputBuffers[i] = new float[(mBufferSizeSamples * properties.mPeriodsPerBuffer) + 32];
    }
    bufferAllocated = true;
    bufferReady = false;
    samplesInBuffers = 0;
    return;
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

