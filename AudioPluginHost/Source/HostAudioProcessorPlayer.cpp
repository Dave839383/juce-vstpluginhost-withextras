/*
  ==============================================================================

    HostAudioProcessorPlayer.cpp
    Created: 18 Mar 2018 4:12:21pm
    Author:  David Lloyd

  ==============================================================================
*/
#include "../JuceLibraryCode/JuceHeader.h"
#include "HostAudioProcessorPlayer.h"
#include "HostPlayHead.h"

HostAudioProcessorPlayer::HostAudioProcessorPlayer (bool doDoublePrecisionProcessing)
: isDoublePrecision (doDoublePrecisionProcessing)
{
    hostPlayHead = new HostPlayHead();
}

HostAudioProcessorPlayer::~HostAudioProcessorPlayer()
{ 
    setProcessor (nullptr);
}

//==============================================================================
void HostAudioProcessorPlayer::setProcessor (AudioProcessor* const processorToPlay)
{
    if (processor != processorToPlay)
    {
        if (processorToPlay != nullptr && sampleRate > 0 && blockSize > 0)
        {
            processorToPlay->setPlayConfigDetails (numInputChans, numOutputChans, sampleRate, blockSize);
            
            bool supportsDouble = processorToPlay->supportsDoublePrecisionProcessing() && isDoublePrecision;
            
            processorToPlay->setProcessingPrecision (supportsDouble ? AudioProcessor::doublePrecision
                                                     : AudioProcessor::singlePrecision);
            processorToPlay->setPlayHead(hostPlayHead);
            hostPlayHead->setSampleRate(sampleRate);
            processorToPlay->prepareToPlay (sampleRate, blockSize);
        }
        
        AudioProcessor* oldOne;
        
        {
            const ScopedLock sl (lock);
            oldOne = isPrepared ? processor : nullptr;
            processor = processorToPlay;
            isPrepared = true;
        }
        
        if (oldOne != nullptr)
            oldOne->releaseResources();
    }
}

void HostAudioProcessorPlayer::setDoublePrecisionProcessing (bool doublePrecision)
{
    if (doublePrecision != isDoublePrecision)
    {
        const ScopedLock sl (lock);
        
        if (processor != nullptr)
        {
            processor->releaseResources();
            
            bool supportsDouble = processor->supportsDoublePrecisionProcessing() && doublePrecision;
            
            processor->setProcessingPrecision (supportsDouble ? AudioProcessor::doublePrecision
                                               : AudioProcessor::singlePrecision);
            processor->prepareToPlay (sampleRate, blockSize);
            hostPlayHead->setSampleRate(sampleRate);
        }
        
        isDoublePrecision = doublePrecision;
    }
}

//==============================================================================
void HostAudioProcessorPlayer::audioDeviceIOCallback (const float** const inputChannelData,
                                                  const int numInputChannels,
                                                  float** const outputChannelData,
                                                  const int numOutputChannels,
                                                  const int numSamples)
{
    // these should have been prepared by audioDeviceAboutToStart()...
    jassert (sampleRate > 0 && blockSize > 0);
    
    incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
    int totalNumChans = 0;
    if (numInputChannels > numOutputChannels)
    {
        // if there aren't enough output channels for the number of
        // inputs, we need to create some temporary extra ones (can't
        // use the input data in case it gets written to)
        tempBuffer.setSize (numInputChannels - numOutputChannels, numSamples,
                            false, false, true);
        
        for (int i = 0; i < numOutputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }
        
        for (int i = numOutputChannels; i < numInputChannels; ++i)
        {
            channels[totalNumChans] = tempBuffer.getWritePointer (i - numOutputChannels);
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }
    }
    else
    {
        for (int i = 0; i < numInputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }
        
        for (int i = numInputChannels; i < numOutputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            zeromem (channels[totalNumChans], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }
    }
    
    AudioBuffer<float> buffer (channels, totalNumChans, numSamples);
    
    {
        const ScopedLock sl (lock);
        
        if (processor != nullptr)
        {
            const ScopedLock sl2 (processor->getCallbackLock());
            
            if (! processor->isSuspended())
            {
                if (hostPlayHead->getIsPlaying())
                {
                    hostPlayHead->setTimeInSamples(numSamples);
                }
                else
                {
                    hostPlayHead->resetTimeInSamples();
                }
                if (processor->isUsingDoublePrecision())
                {
                    conversionBuffer.makeCopyOf (buffer, true);
                    processor->processBlock (conversionBuffer, incomingMidi);
                    buffer.makeCopyOf (conversionBuffer, true);
                }
                else
                {
                    // incomingMidi = MidiBuffer object.
                    processor->processBlock (buffer, incomingMidi);
                }
                
                if (! incomingMidi.isEmpty() && midiOutput != nullptr)
                    midiOutput->sendBlockOfMessages (incomingMidi, Time::getMillisecondCounter(), processor->getSampleRate());
                
                return;
            }
        }
    }
    
    for (int i = 0; i < numOutputChannels; ++i)
        FloatVectorOperations::clear (outputChannelData[i], numSamples);
}

void HostAudioProcessorPlayer::audioDeviceAboutToStart (AudioIODevice* const device)
{
    auto newSampleRate = device->getCurrentSampleRate();
    auto newBlockSize  = device->getCurrentBufferSizeSamples();
    auto numChansIn    = device->getActiveInputChannels().countNumberOfSetBits();
    auto numChansOut   = device->getActiveOutputChannels().countNumberOfSetBits();
    
    const ScopedLock sl (lock);
    
    sampleRate = newSampleRate;
    hostPlayHead->setSampleRate(sampleRate);
    blockSize  = newBlockSize;
    numInputChans  = numChansIn;
    numOutputChans = numChansOut;
    
    messageCollector.reset (sampleRate);
    channels.calloc (jmax (numChansIn, numChansOut) + 2);
    
    if (processor != nullptr)
    {
        if (isPrepared)
            processor->releaseResources();
        
        auto* oldProcessor = processor;
        setProcessor (nullptr);
        setProcessor (oldProcessor);
    }
}

void HostAudioProcessorPlayer::audioDeviceStopped()
{
    const ScopedLock sl (lock);
    
    if (processor != nullptr && isPrepared)
        processor->releaseResources();
    
    sampleRate = 0.0;
    hostPlayHead->setSampleRate(sampleRate);
    blockSize = 0;
    isPrepared = false;
    tempBuffer.setSize (1, 1);
}

void HostAudioProcessorPlayer::handleIncomingMidiMessage (MidiInput*, const MidiMessage& message)
{
    messageCollector.addMessageToQueue (message);
}

void HostAudioProcessorPlayer::setMidiOutput (MidiOutput* newMidiOutput)
{
    if (midiOutput != newMidiOutput)
    {
        const ScopedLock sl (lock);
        
        midiOutput = newMidiOutput;
    }
}

bool HostAudioProcessorPlayer::getIsPlaying()
{
    return hostPlayHead->getIsPlaying();
}

void HostAudioProcessorPlayer::setIsPlaying(bool _isPlaying)
{
    hostPlayHead->setIsPlaying(_isPlaying);
}

void HostAudioProcessorPlayer::setBpm(double _bpm)
{
    hostPlayHead->setBpm(_bpm);
}

