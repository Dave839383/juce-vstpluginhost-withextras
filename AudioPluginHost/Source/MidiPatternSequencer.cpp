/*
  ==============================================================================

    MidiPatternSequencer.cpp
    Created: 20 Mar 2018 7:35:41pm
    Author:  David Lloyd

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MidiPatternSequencer.h"

MidiPatternSequencer::MidiPatternSequencer() : octave(4)
{
    // set all notes in pattern to 'off' by default, ie. -1.
    for (int i = 0; i < 16; i++)
    {
        notesArray.add(-1);
    }
}

int MidiPatternSequencer::getOctave()
{
    return octave;
}

void MidiPatternSequencer::moveOneOctaveHigher()
{
    octave += 1;
    for (int i = 0; i < 16; i++)
    {
        if (notesArray[i] != -1 && notesArray[i] + 12 <= 127)
        {
            notesArray.set(i, notesArray[i] + 12);
        }
    }
}

void MidiPatternSequencer::moveOneOctaveLower()
{
    octave -= 1;
    for (int i = 0; i < 16; i++)
    {
        if (notesArray[i] != -1 && notesArray[i] - 12 >= 0)
        {
            notesArray.set(i, notesArray[i] - 12);
        }
    }
}

void MidiPatternSequencer::fillInPluginDescription (PluginDescription& d) const
{
    d.name = getName();
    d.uid = d.name.hashCode();
    d.category = "Audio Midi devices";
    d.pluginFormatName = "Internal";
    d.manufacturerName = "David's Software";
    d.version = "1.0";
    d.isInstrument = true;
    
    d.numInputChannels = getTotalNumInputChannels();
    d.numOutputChannels = getTotalNumOutputChannels();
}

const String MidiPatternSequencer::getName() const
{
    return "Midi Pattern Sequencer";
}

void MidiPatternSequencer::releaseResources()
{
    // keyboardState.reset();
}

void MidiPatternSequencer::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;
    currentStep = 0;
    prevNote = 0;
    
    // removes all events, clears memory somehow, allNotesOff needs to be called to turn all notes off though.
    keyboardState.reset();
    keyboardState.allNotesOff(0);
}

void MidiPatternSequencer::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi)
{
    auto numSamples = buffer.getNumSamples();
    
    updateCurrentTimeInfoFromHost();

    double beatLengthInSamples = 60.0 / (lastPosInfo.bpm * 4) * currentSampleRate;
    
    int beatLengthMultiple = (int)lastPosInfo.timeInSamples % (int)beatLengthInSamples;

    // if the current beatMultiple falls between these values it means we are up to a point where we should play a sequence step.  Note: this must be >= on one side and < on the other side because otherwise, if a beatLengthMultiple = 0, after the very next 64/128/512 etc samples come through it will trigger a note again.
    if (beatLengthMultiple >= 0 && beatLengthMultiple < numSamples && lastPosInfo.isPlaying)
    {
        if (prevNote != -1)
        {
            keyboardState.noteOff(1, prevNote, 0.5f);
        }
        
        if (notesArray[currentStep] != -1)
        {
            keyboardState.noteOn(1, notesArray[currentStep], 0.5f);
        }
        prevNote = notesArray[currentStep];
        if (currentStep + 1 > 15)
        {
            currentStep = 0;
        }
        else
        {
            currentStep += 1;
        }
    }
    else if (!lastPosInfo.isPlaying)
    {
        keyboardState.allNotesOff(0);
        prevNote = 0;
        currentStep = 0;
    }
    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);
}

void MidiPatternSequencer::updateCurrentTimeInfoFromHost()
{
    if (auto* ph = getPlayHead())
    {
        AudioPlayHead::CurrentPositionInfo newTime;
        
        if (ph->getCurrentPosition (newTime))
        {
            lastPosInfo = newTime;  // Successfully got the current time from the host..
            return;
        }
    }
    
    // If the host fails to provide the current time, we'll just reset our copy to a default..
    lastPosInfo.resetToDefault();
}

bool MidiPatternSequencer::hasEditor() const
{
    return true;
}

bool MidiPatternSequencer::checkStepIsOn(const int& i)
{
    jassert(i < 16);
    
    if (notesArray[i] != -1)
    {
        return true;
    }
    return false;
}

int MidiPatternSequencer::getNoteForStep(const int& step)
{
    // the note array must always be 16 notes long
    jassert(step < 16);
    
    if (notesArray[step] != -1)
    {
        return notesArray[step];
    }
    return -1;
}

void MidiPatternSequencer::addNoteToPattern(const int& step, const int& note)
{
    jassert(step < 16);
    
    jassert(note < 128);

    notesArray.set(step, note);
}

void MidiPatternSequencer::deleteNoteFromPattern(const int& step)
{
    jassert(step < 16);
    
    notesArray.set(step, -1);
}

AudioProcessorEditor* MidiPatternSequencer::createEditor()
{
    return new MidiPatternSequencerEditor (*this);
}

// Editor Window Functions
MidiPatternSequencerEditor::MidiPatternSequencerEditor(MidiPatternSequencer& owner) : AudioProcessorEditor (owner),
    octaveButtonLow("octaveButtonLow", DrawableButton::ImageFitted),
    octaveButtonHigh("octaveButtonHigh", DrawableButton::ImageFitted)
{
    addAndMakeVisible (octaveButtonLow);
    octaveButtonLow.addListener (this);
    
    addAndMakeVisible (octaveButtonHigh);
    octaveButtonHigh.addListener (this);
    
    setOctaveLabel(getProcessor().getOctave());
    noteName.setColour(Label::textColourId, Colours::black);
    addAndMakeVisible (noteName);
    noteName.addListener(this);
    
    for (int i = 1; i < 193; i++)
    {
        TextButton * t = new TextButton();
        // this is reloading all the button with blank values by default, you need to override this to store the correct button values if they exist.
        t->setComponentID(String(i));
        t->setColour(TextButton::buttonOnColourId, Colours::yellow);
        t->setName("patternSequenceButton");
        addAndMakeVisible (t);
        t->addListener (this);
        patternButtonArray.add(t);
    }
    
    setSize(300, 200);
}

void MidiPatternSequencerEditor::buttonClicked (Button* button)
{
    int currentOctave = getProcessor().getOctave();
    if (button->getName() == "patternSequenceButton")
    {
        for (int i = 0; i < patternButtonArray.size(); i++)
        {
            if (button == patternButtonArray[i])
            {
                // check if this step already has a note entered and if it does delete it and replace it with this new note.
                int step = i % 16;
                int note = i / 16;
                if (getProcessor().checkStepIsOn(step) && !patternButtonArray[i]->getToggleState())
                {
                    clearStepColumn(step);
                }
                
                if (patternButtonArray[i]->getToggleState())
                {
                    patternButtonArray[i]->setToggleState(false, NotificationType::dontSendNotification);
                    getProcessor().deleteNoteFromPattern(step);
                }
                else
                {
                    patternButtonArray[i]->setToggleState(true, NotificationType::dontSendNotification);
                    getProcessor().addNoteToPattern(step, currentOctave * 12 + note);
                }
                break;
            }
        }
    }
    else if (button->getName() == "octaveButtonLow")
    {
        if (currentOctave > 0)
        {
            getProcessor().moveOneOctaveLower();
            setOctaveLabel(currentOctave - 1);
        }
    }
    else if (button->getName() == "octaveButtonHigh")
    {
        if (currentOctave < 10)
        {
            getProcessor().moveOneOctaveHigher();
            setOctaveLabel(currentOctave + 1);
        }
    }
}

void MidiPatternSequencerEditor::setOctaveLabel(const int& oct)
{
    String currentOctave = (String)"C" += (oct - 2);
    noteName.setText(currentOctave, NotificationType::sendNotification);
}

void MidiPatternSequencerEditor::labelTextChanged (Label * labelThatHasChanged)
{

}

void MidiPatternSequencerEditor::paint (Graphics& g)
{
    g.setColour(Colour::fromRGB(203, 203, 203));
    g.fillAll();
}

void MidiPatternSequencerEditor::resized()
{
    int border = 20;
    int patternAreaEdge = 40;
    Rectangle<int> patternArea(patternAreaEdge, border, getWidth() - border - patternAreaEdge, getHeight() - (border * 2));
    int patternColumn = patternArea.getWidth() / 16;
    int patternRow = patternArea.getHeight() / 12;
    
    // set up 16 * 12 buttons.
    for (int i = 0; i < 12; i++)
    {
        int ioctave = i * 16;
        auto currentRowArea = patternArea.removeFromBottom(patternRow);
        for (int j = 0; j < 16; j++)
        {
            int note = getProcessor().getNoteForStep(j);
            // if not -1, there is a note for this step.
            if (note != -1)
            {
                // note / 12 gives you the note row
                if (note % 12 == i)
                {
                    patternButtonArray[j +ioctave]->setToggleState(true, NotificationType::dontSendNotification);
                }
            }
            patternButtonArray[j + ioctave]->setBounds(currentRowArea.removeFromLeft(patternColumn));
        }
    }
    
    // set up downward pointing triangle that can be used to move pattern down an octave.
    DrawablePath octaveLowDrawablePath, octaveHighDrawablePath;
    
    // this is the colour of the octave buttons
    FillType f(getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    Path octaveLowPath;
    octaveLowPath.addTriangle(0, 0, 15, 0, 7.5, 15);
    octaveLowDrawablePath.setPath(octaveLowPath);
    octaveLowDrawablePath.setFill(f);
    octaveLowPath.createPathWithRoundedCorners(0.8f);

    octaveButtonLow.setImages(&octaveLowDrawablePath);
    octaveButtonLow.setEdgeIndent(1);
    octaveButtonLow.setBounds(getWidth() - border, getHeight() - border - 1, 12, 12);
    
    Path octaveHighPath;
    octaveHighPath.addTriangle(0, 15, 7.5, 0, 15, 15);
    octaveHighDrawablePath.setPath(octaveHighPath);
    octaveHighDrawablePath.setFill(f);
    octaveButtonHigh.setImages(&octaveHighDrawablePath);
    octaveButtonHigh.setEdgeIndent(1);
    octaveButtonHigh.setBounds(getWidth() - border, getHeight() - border - 12 - 1, 12, 12);

    noteName.setBounds(patternAreaEdge - 20, getHeight() - border - 20, 20, 20);
    BorderSize<int> b(1);
    noteName.setBorderSize(b);
    noteName.setJustificationType(Justification::bottomLeft);
}

void MidiPatternSequencerEditor::clearStepColumn(const int& step)
{
    for (int i = 0; i < 12; i++)
    {
        int currentStep = i * 16 + step;
        if (patternButtonArray[currentStep]->getToggleState())
        {
            patternButtonArray[currentStep]->setToggleState(false, NotificationType::dontSendNotification);
            return;
        }
    }
}
