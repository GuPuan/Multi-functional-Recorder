#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MFRecorderProcessorEditor::MFRecorderProcessorEditor(MFRecorderProcessor& p)
    : AudioProcessorEditor(&p), Processor(p)
{
    setSize (400,350);

    addAndMakeVisible(Processor.waveViewer);
    Processor.waveViewer.setColours(juce::Colours::black,juce::Colours::whitesmoke);

    addAndMakeVisible(recordButton);
    recordButton.setButtonText("RecordModel");
    recordButton.setColour(recordButton.buttonColourId, juce::Colours::green);
    recordButton.setEnabled(true);
    recordButton.setClickingTogglesState(false);
    recordButton.onClick = [this] {modelButtonClick();};

    addAndMakeVisible(openButton);
    openButton.onClick = [this] { openButtonClick(); };
    openButton.setButtonText("Open Audio File");

    addAndMakeVisible(playButton);
    playButton.onClick = [this] { playButtonClick(); };
    playButton.setColour(playButton.buttonColourId, juce::Colours::darkgreen);
    playButton.setButtonText("START");
    playButton.setEnabled(true);

    addAndMakeVisible(stopButton);
    stopButton.onClick = [this] {stopButtonClick(); };
    stopButton.setColour(stopButton.buttonColourId, juce::Colours::maroon);
    stopButton.setButtonText("STOP");
    stopButton.setEnabled(false);
}

MFRecorderProcessorEditor::~MFRecorderProcessorEditor()
{
}

//==============================================================================
void MFRecorderProcessorEditor::paint (juce::Graphics& g)
{
    
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MFRecorderProcessorEditor::resized()
{
    int topMargin = 20; // Top margin for the first component
    int sideMargin = 20; // Side margins for all components (left and right)
    int componentHeight = 40; // Height for each component
    int spacing = 10; // Vertical spacing between components
    int buttonSpacing = 10; // Horizontal spacing between recordButton and openButton

    int width = getWidth() - 2 * sideMargin; // Compute the width for components, taking into account the side margins
    int halfWidth = (width - buttonSpacing) / 2; // Calculate the width for side-by-side buttons, considering the space between them

    // Adjust the position and size of the wave viewer
    Processor.waveViewer.setBounds(sideMargin, topMargin, width, componentHeight * 4);

    // Calculate the top position for the next component following the wave viewer and its spacing
    int nextTop = topMargin + componentHeight * 4 + spacing;

    // Set the position and size for the recordButton
    recordButton.setBounds(sideMargin, nextTop, halfWidth, componentHeight);

    // Set the position and size for the openButton, placing it to the right of the recordButton
    openButton.setBounds(sideMargin + halfWidth + buttonSpacing, nextTop, halfWidth, componentHeight);

    // Update nextTop for the top position of the following component
    nextTop += componentHeight + spacing;

    // Set the position and size for the playButton
    playButton.setBounds(sideMargin, nextTop, width, componentHeight);

    // Update nextTop for the top position of the next component
    nextTop += componentHeight + spacing;

    // Set the position and size for the stopButton
    stopButton.setBounds(sideMargin, nextTop, width, componentHeight);
    // Continue this pattern for any additional components
}

void MFRecorderProcessorEditor::modelButtonClick() {
    modelState = Processor.modelChange();
    if (modelState) 
        recordButton.setButtonText("RecordModel");
    else
        recordButton.setButtonText("PlayModel");
}

void MFRecorderProcessorEditor::openButtonClick()
{ 
    addAudioFile();
}

void MFRecorderProcessorEditor::playButtonClick()
{
    Processor.changeState(MFRecorderProcessor::TransportState::Starting);
    playButton.setEnabled(false);
    stopButton.setEnabled(true);

    if (modelState)
    {
        juce::FileChooser chooser("Save audio file", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav");
        if (chooser.browseForFileToSave(true)) {
            myFile = chooser.getResult();
            DBG("Created " << myFile.getFullPathName());
        }
        startRecording();
    }
}

void MFRecorderProcessorEditor::stopButtonClick()
{
    Processor.changeState(MFRecorderProcessor::TransportState::Stopping);
    stopButton.setEnabled(false);
    playButton.setEnabled(true);

    if (Processor.isRecording()) stopRecording();
}

void MFRecorderProcessorEditor::addAudioFile()
{
    chooser = std::make_unique<juce::FileChooser> ("Select an audio file...", juce::File{}, "*.wav", true);

    auto fileChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;


    chooser->launchAsync(fileChooserFlags, [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file.existsAsFile())
            {
                audioSource = file;
                Processor.loadFile(audioSource);

                juce::String labelText = "Audio File Loaded: " + audioSource.getFileName();
                audioSourceLabel.setText(labelText, juce::NotificationType::dontSendNotification);

                juce::MessageManager::callAsync([this]()
                    {
                        playButton.setEnabled(true);
                    });
            }
            else if (!file.exists())
            {
            }
        });
}

void MFRecorderProcessorEditor::startRecording() {
    lastRecording = myFile;
    Processor.startRecording(lastRecording);
}

void MFRecorderProcessorEditor::stopRecording() {
    Processor.stop();
}


