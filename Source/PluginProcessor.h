#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class MFRecorderProcessor  : public juce::AudioProcessor, public juce::ChangeListener
{
public:

    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    juce::AudioVisualiserComponent waveViewer;

    //==============================================================================
    MFRecorderProcessor();
    ~MFRecorderProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void changeState(TransportState newState);
    void loadFile(juce::File &filename);

    void startRecording(const File& file);
    void stop();
    bool isRecording();
    bool modelChange();

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MFRecorderProcessor)
    //========== Play File Part =========//
    bool modelSetting = true;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;
    //========== Recorder Part =========//

    TimeSliceThread backgroundThread{ "Audio Recorder Thread" };
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter;
    double sampleRate = 0.0;
    CriticalSection writerLock;
    std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //====================//
};
