/*
  ==============================================================================

    MidiPatternSequencer.h
    Created: 20 Mar 2018 7:35:41pm
    Author:  David Lloyd

  ==============================================================================
*/

#pragma once

class MidiPatternSequencer : public AudioPluginInstance
{
public:
    //==============================================================================
    MidiPatternSequencer();
    ~MidiPatternSequencer() {
        
    }
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    
    void releaseResources() override;
    
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midi) override;
    
    void fillInPluginDescription (PluginDescription& d) const override;
    
    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    const String getName() const override;
    
    bool acceptsMidi() const override                      { return true; }
    bool producesMidi() const override                     { return true; }
    double getTailLengthSeconds() const override           { return 0; }
    
    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const String&) override   {}
    
    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        //MemoryOutputStream (destData, true).writeFloat (*speed);
    }
    
    void setStateInformation (const void* data, int sizeInBytes) override
    {
        //speed->setValueNotifyingHost (MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
    }
    
    // takes an integer between 0 and 15 and checks if that step is currently used by a note or not in the pattern.
    bool checkStepIsOn(const int& i);
    
    int getNoteForStep(const int& step);

    // this keeps a copy of the last set of time info that was acquired during an audio
    // callback - the UI component will read this and display it.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;
    
    void addNoteToPattern(const int& step, const int& note);
    void deleteNoteFromPattern(const int& step);
    
    // returns the current octave associated with this pattern.
    int getOctave();
    void setOctave(const int& oct);
    
    void moveOneOctaveHigher();
    void moveOneOctaveLower();
private:
    void updateCurrentTimeInfoFromHost();
    
    // holds the current octave selected by the octave ShapeButtons
    int octave;
    
    // my stuff
    Array<int> notesArray;
    int currentSampleRate;
    
    // stores the nextStep due to be played.
    int currentStep;
    int prevNote;
    
    MidiKeyboardState keyboardState;
};

class MidiPatternSequencerEditor  : public AudioProcessorEditor,
                                    public Button::Listener,
                                    public Label::Listener
{
public:
    MidiPatternSequencerEditor(MidiPatternSequencer& owner);
    
    void buttonClicked (Button* button) override;
    
    void labelTextChanged (Label * labelThatHasChanged) override;
private:
    void paint (Graphics& g) override;
    
    //==============================================================================
    MidiPatternSequencer& getProcessor() const
    {
        return static_cast<MidiPatternSequencer&> (processor);
    }
    
    void resized() override;
    
    // turns all buttons off for a given step column int.
    void clearStepColumn(const int& step);
    
    // this array will own any objects if they're passed using new and will delete them.
    OwnedArray<TextButton> patternButtonArray;
    
    DrawableButton octaveButtonLow;
    DrawableButton octaveButtonHigh;
    
    // the note name that is listed on the bottom left side of the pattern matrix.
    Label noteName;
    
    // sets the new text in the octave label
    void setOctaveLabel(const int& oct);
};


