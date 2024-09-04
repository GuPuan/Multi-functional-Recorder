#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MFRecorderProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MFRecorderProcessorEditor (MFRecorderProcessor&);
    ~MFRecorderProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
     
    MFRecorderProcessor& Processor;

    //================  PlayFile Part  =======================//
    juce::TextButton recordButton;
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::Label audioSourceLabel;
    std::unique_ptr<juce::FileChooser> chooser;
    
    juce::File audioSource;

    void openButtonClick();
    void playButtonClick();
    void stopButtonClick();
    void addAudioFile();
    void modelButtonClick();

    //================  Recorder Part  =======================//
    juce::File lastRecording;
    juce::File myFile;

    void startRecording();
    void stopRecording();

    bool modelState=true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MFRecorderProcessorEditor)
};
