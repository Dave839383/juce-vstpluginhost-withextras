/*
  ==============================================================================

    HostPlayHead.h
    Created: 18 Mar 2018 5:10:18pm
    Author:  David Lloyd

  ==============================================================================
*/

#pragma once

class HostPlayHead   : public AudioPlayHead
{
public:
    HostPlayHead();
    ~HostPlayHead() {}
    
    void resetTimeInSamples();
    void setTimeInSamples(int t);
    bool getCurrentPosition (CurrentPositionInfo &result) override;
    void setIsPlaying(bool isPlaying);
    bool getIsPlaying();
    void setSampleRate(double _sampleRate);
    double getSampleRate();
    
    void setBpm(double bpm);
    
private:
    CurrentPositionInfo c;
    double sampleRate = 44100.0;
    double secondsPerBeat = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostPlayHead)
};
