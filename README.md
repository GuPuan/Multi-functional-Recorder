# MF Recorder Plugin

## Overview
The **MF Recorder** is a multifunctional recording plugin developed using the JUCE framework. It offers a unique feature that allows users to record new audio on top of existing audio files, enabling a multi-track recording experience. The plugin also features a playback-only mode for easy audio review.

## Features
- **Two Modes**:
  - **Recording Mode**: Allows users to layer new recordings over existing audio files, creating multiple tracks in one recording session.
  - **Playback Mode**: Simply plays selected audio files without recording new audio.
- **Buffer Management**: The plugin uses a temporary buffer to overlay new audio onto existing files.
- **Dynamic Waveform Display**: The UI includes an oscilloscope to display live waveforms during recording and playback.

## How It Works
- In **Recording Mode**, the plugin allows users to load an existing audio file, record new audio over it, and save the combined file. Users can also record without selecting an existing file.
- In **Playback Mode**, users can load an audio file and play it without recording.
- The plugin supports seamless switching between the two modes via the UI.
  
## Installation
1. Clone or download the repository.
2. Open the project in your JUCE environment.
3. Compile the project and use the recorder in your preferred DAW as a VST3 plugin.

## Usage
- **Recording Mode**: Load an existing audio file or start a new recording. New audio is layered on top of existing tracks.
- **Playback Mode**: Select an audio file to play without recording.
- The waveform display provides real-time visualization of the audio being processed.
