# juce-vstpluginhost-withextras
The aim of this project is to add new features to the JUCE Audio Plugin Host.  The JUCE Audio Plugin Host is used for testing VST/AU Plugins built using the JUCE C++ Library, and these features will allow the user to test with a working AudioPlayHead before moving the plugin to Logic/Pro Tools etc.

See https://github.com/Dave839383/juce-vstpluginhost-withextras/wiki for screenshots

At the moment Playback controls (ie. play/stop buttons, bpm control), Midi Out and a Midi Pattern Sequencer have been added.  

The Play and Stop buttons can play and stop an AudioPlayHead that any plugin added to the host can pick up and use.

The Midi Pattern Sequencer is a built in AudioPluginInstance which can send a pattern of 16 16th notes via midi out to any plugin that is able to be connected to it.  It has an easy to use UI where notes can be entered via a 12 * 16 note matrix.  The current octave of the matrix can be moved up or down in real-time.

This is an ongoing project and more features will be added.

### How to run on MAC:

1. Download JUCE from https://shop.juce.com/get-juce or https://github.com/WeAreROLI/JUCE
2. open the file juce-vstpluginhost-withextras/AudioPluginHost.jucer and set the modules folder to the modules folder in the main JUCE folder.
3. open with XCode and build.
