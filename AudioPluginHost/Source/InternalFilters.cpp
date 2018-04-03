/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "InternalFilters.h"
#include "MidiPatternSequencer.h"
#include "FilterGraph.h"

//==============================================================================
InternalPluginFormat::InternalPluginFormat()
{
    {
        // constructor which takes in an AudioProcessorGraph::AudioGraphIOProcessor::IODeviceType
        // AudioProcessorGraph::AudioGraphIOProcessor is an AudioPluginInstance
        // Each of these are only created to fill the plugin description of audioOutDesc, audioInDesc etc
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
        p.fillInPluginDescription (audioOutDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
        p.fillInPluginDescription (audioInDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
        p.fillInPluginDescription (midiInDesc);
    }
    
    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode);
        p.fillInPluginDescription (midiOutDesc);
    }
    
    {
        // this was not part of the original JUCE audio plugin host.  It is a new MidiPatternSequencer that can send MIDI to other instruments.
        MidiPatternSequencer m;
        m.fillInPluginDescription (midiDesc);
    }
}

AudioPluginInstance* InternalPluginFormat::createInstance (const String& name)
{
    // audioOutDesc is a PluginDescription, filled-in in constructor.
    // AudioGraphIOProcessor is an AudioPluginInstance.
    if (name == audioOutDesc.name) return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
    if (name == audioInDesc.name)  return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
    if (name == midiInDesc.name)   return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
    if (name == midiOutDesc.name) return new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode);
    if (name == midiDesc.name) return new MidiPatternSequencer;

    return nullptr;
}

void InternalPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                 double /*initialSampleRate*/,
                                                 int /*initialBufferSize*/,
                                                 void* userData,
                                                 void (*callback) (void*, AudioPluginInstance*, const String&))
{
    auto* p = createInstance (desc.name);

    callback (userData, p, p == nullptr ? NEEDS_TRANS ("Invalid internal filter name") : String());
}

bool InternalPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept
{
    return false;
}

void InternalPluginFormat::getAllTypes (OwnedArray<PluginDescription>& results)
{
    results.add (new PluginDescription (audioInDesc));
    results.add (new PluginDescription (audioOutDesc));
    results.add (new PluginDescription (midiInDesc));
    results.add (new PluginDescription (midiOutDesc));
    results.add (new PluginDescription (midiDesc));
}
