#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MFRecorderProcessor::MFRecorderProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), waveViewer(1)
#endif
{
    formatManager.registerBasicFormats(); 
    transportSource.addChangeListener(this); 
    backgroundThread.startThread();
    waveViewer.setRepaintRate(30);
    waveViewer.setBufferSize(512);
}

MFRecorderProcessor::~MFRecorderProcessor()
{
    stop();
}

//==============================================================================
const juce::String MFRecorderProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MFRecorderProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MFRecorderProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MFRecorderProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MFRecorderProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MFRecorderProcessor::getNumPrograms()
{
    return 1; 
}

int MFRecorderProcessor::getCurrentProgram()
{
    return 0;
}

void MFRecorderProcessor::setCurrentProgram (int index)
{
}

const juce::String MFRecorderProcessor::getProgramName (int index)
{
    return {};
}

void MFRecorderProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MFRecorderProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    this->sampleRate = sampleRate;
    waveViewer.clear();
}

void MFRecorderProcessor::releaseResources()
{
    transportSource.releaseResources();
    this->sampleRate = 0;
    waveViewer.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MFRecorderProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}
#endif

void MFRecorderProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const ScopedLock sl(writerLock);

    if (modelSetting)
    {
        if (activeWriter.load() != nullptr)
        {
            juce::AudioBuffer<float> fileBuffer(totalNumOutputChannels, buffer.getNumSamples());
            juce::AudioSourceChannelInfo audioSourceChannelInfo(fileBuffer);
            transportSource.getNextAudioBlock(audioSourceChannelInfo);
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                if (channel < totalNumInputChannels)
                {
                    buffer.addFrom(channel, 0, fileBuffer, channel, 0, buffer.getNumSamples());
                }
            }
            activeWriter.load()->write(buffer.getArrayOfReadPointers(), buffer.getNumSamples());
            waveViewer.pushBuffer(buffer);
        }
    }
    else
    {
        juce::AudioBuffer<float> fileBuffer(totalNumOutputChannels, buffer.getNumSamples());
        juce::AudioSourceChannelInfo audioSourceChannelInfo(fileBuffer);
        transportSource.getNextAudioBlock(audioSourceChannelInfo);
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            if (channel < totalNumInputChannels)
            {
                buffer.addFrom(channel, 0, fileBuffer, channel, 0, buffer.getNumSamples());
            }
        }
        waveViewer.pushBuffer(buffer);
    }
}

//==============================================================================
bool MFRecorderProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MFRecorderProcessor::createEditor()
{
    return new MFRecorderProcessorEditor (*this);
}

//==============================================================================
void MFRecorderProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void MFRecorderProcessor::setStateInformation (const void* data, int sizeInBytes)
{
   
}

void MFRecorderProcessor::changeState(TransportState newState)
{
    if (state != newState)
    {
        state = newState;
        switch (state)
        {
            case Stopped:
                transportSource.setPosition(0.0);
                break;
            case Starting:
                transportSource.start();
                break;
            case Stopping:
                transportSource.stop();
                break;
        }
    }
}

void MFRecorderProcessor::loadFile(juce::File &filename)
{
    auto reader = formatManager.createReaderFor(filename);
    if (reader != nullptr)
    {
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release()); 
    }
    else
    {
        DBG("Failed to load file: " << filename.getFullPathName());
    }
}

void MFRecorderProcessor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource) 
    {
        if (transportSource.isPlaying())
        {
            changeState(TransportState::Playing); 
        }
        else
        {
            changeState(TransportState::Stopped); 
        }
    }
}

void MFRecorderProcessor::startRecording(const File& file)
{
    stop();
    if (sampleRate > 0)
    {
        file.deleteFile();
        if (auto fileStream = std::unique_ptr<FileOutputStream>(file.createOutputStream()))
        {
            WavAudioFormat wavFormat;
            if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, getTotalNumInputChannels(), 24, {}, 0))
            {
                fileStream.release(); 
                threadedWriter.reset(new AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));
                const ScopedLock sl(writerLock);
                activeWriter = threadedWriter.get();
            }
        }
    }
}

void MFRecorderProcessor::stop()
{
    {
        const ScopedLock sl(writerLock);
        activeWriter = nullptr;
    }   
    threadedWriter.reset();
}

bool MFRecorderProcessor::isRecording()
{
    return activeWriter.load() != nullptr;
}

bool MFRecorderProcessor::modelChange() {
    if (!modelSetting) modelSetting = true;
    else modelSetting = false;
    return modelSetting;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MFRecorderProcessor();
}
