/*
  ==============================================================================

    HostPlayHead.cpp
    Created: 18 Mar 2018 5:10:18pm
    Author:  David Lloyd

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "HostPlayHead.h"

HostPlayHead::HostPlayHead() {
    c.bpm = 120;
    secondsPerBeat = 120.0 / 60.0 / 4.0;
    c.timeSigNumerator = 4;
    c.timeSigDenominator = 4;
    c.timeInSamples = 0;
    c.isPlaying = false;
}

void HostPlayHead::setTimeInSamples(int t)
{
    c.timeInSamples += t;
}

void HostPlayHead::resetTimeInSamples()
{
    c.timeInSamples = 0;
}

bool HostPlayHead::getCurrentPosition (CurrentPositionInfo &result)
{
    result.bpm = c.bpm;
    result.timeSigNumerator = c.timeSigNumerator;
    result.timeSigDenominator = c.timeSigDenominator;
    result.timeInSamples = c.timeInSamples;
    result.timeInSeconds = c.timeInSamples / getSampleRate();
    result.ppqPosition = result.timeInSeconds / secondsPerBeat;
    result.isPlaying = c.isPlaying;
    return true;
}

void HostPlayHead::setIsPlaying(bool isPlaying)
{
    c.isPlaying = isPlaying;
}

bool HostPlayHead::getIsPlaying()
{
    return c.isPlaying;
}

void HostPlayHead::setSampleRate(double _sampleRate)
{
    sampleRate = _sampleRate;
}

double HostPlayHead::getSampleRate()
{
    return sampleRate;
}

void HostPlayHead::setBpm(double bpm)
{
    c.bpm = bpm;
}

