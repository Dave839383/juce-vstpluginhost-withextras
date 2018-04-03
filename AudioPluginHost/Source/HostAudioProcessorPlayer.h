/*
  ==============================================================================

    HostAudioProcessorPlayer.h
    Created: 18 Mar 2018 4:12:21pm
    Author:  David Lloyd

  ==============================================================================
*/

#pragma once

#include "HostPlayHead.h"

    //==============================================================================
    /**
     An AudioIODeviceCallback object which streams audio through an AudioProcessor.
     
     To use one of these, just make it the callback used by your AudioIODevice, and
     give it a processor to use by calling setProcessor().
     
     It's also a MidiInputCallback, so you can connect it to both an audio and midi
     input to send both streams through the processor.
     
     @see AudioProcessor, AudioProcessorGraph
     
     @tags{Audio}
     */
class HostAudioProcessorPlayer    : public AudioIODeviceCallback,
                                    public MidiInputCallback
{
public:
    //==============================================================================
    HostAudioProcessorPlayer (bool doDoublePrecisionProcessing = false);
    
    /** Destructor. */
    virtual ~HostAudioProcessorPlayer();
    
    //==============================================================================
    /** Sets the processor that should be played.
     
     The processor that is passed in will not be deleted or owned by this object.
     To stop anything playing, pass a nullptr to this method.
     */
    void setProcessor (AudioProcessor* processorToPlay);
    
    /** Returns the current audio processor that is being played. */
    AudioProcessor* getCurrentProcessor() const noexcept            { return processor; }
    
    /** Returns a midi message collector that you can pass midi messages to if you
     want them to be injected into the midi stream that is being sent to the
     processor.
     */
    MidiMessageCollector& getMidiMessageCollector() noexcept        { return messageCollector; }
    
    /** Switch between double and single floating point precisions processing.
     The audio IO callbacks will still operate in single floating point
     precision, however, all internal processing including the
     AudioProcessor will be processed in double floating point precision if
     the AudioProcessor supports it (see
     AudioProcessor::supportsDoublePrecisionProcessing()).
     Otherwise, the processing will remain single precision irrespective of
     the parameter doublePrecision. */
    void setDoublePrecisionProcessing (bool doublePrecision);
    
    /** Returns true if this player processes internally processes the samples with
     double floating point precision. */
    inline bool getDoublePrecisionProcessing() { return isDoublePrecision; }
    
    //==============================================================================
    /** @internal */
    void audioDeviceIOCallback (const float**, int, float**, int, int) override;
    /** @internal */
    void audioDeviceAboutToStart (AudioIODevice*) override;
    /** @internal */
    void audioDeviceStopped() override;
    /** @internal */
    void handleIncomingMidiMessage (MidiInput*, const MidiMessage&) override;
    
    /** courtesy of https://forum.juce.com/t/adding-midi-out-to-vst-host-demo/19442/6 Rail_Jon_Rogut **/
    void setMidiOutput (MidiOutput* newMidiOutput);
    
    /*
     * new functions added by me, rest were taken from AudioProcessorPlayer class.
     * all these functions set/get basic variables in the AudioPlayHead.
     */
    void setIsPlaying(bool _isPlaying);
    bool getIsPlaying();
    void setBpm(double _bpm);
    
private:
    //==============================================================================
    AudioProcessor* processor = nullptr;
    CriticalSection lock;
    double sampleRate = 0;
    int blockSize = 0;
    bool isPrepared = false, isDoublePrecision = false;
    
    int numInputChans = 0, numOutputChans = 0;
    HeapBlock<float*> channels;
    AudioBuffer<float> tempBuffer;
    AudioBuffer<double> conversionBuffer;
    
    MidiBuffer incomingMidi;
    MidiMessageCollector messageCollector;
    
    MidiOutput* midiOutput = nullptr;
    
    ScopedPointer<HostPlayHead> hostPlayHead;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostAudioProcessorPlayer)
};
