/*
  ==============================================================================

    PlaybackComponent.h
    Created: 18 Mar 2018 6:27:04pm
    Author:  David Lloyd
 
    Plays back a HostAudioProcessorPlayer.

  ==============================================================================
*/

#pragma once
#include "HostAudioProcessorPlayer.h"

class PlaybackLookAndFeel : public LookAndFeel_V4
{
public:
    PlaybackLookAndFeel();
    
    void drawLabel (Graphics& g, Label& label);
};

class PlaybackComponent   : public Component,
                            public Button::Listener,
                            public Slider::Listener
{
    public:
        PlaybackComponent(HostAudioProcessorPlayer& hpp);
        ~PlaybackComponent() {}
    
        void paint (Graphics& g) override;
    
        void resized() override;
    
        void buttonClicked (Button* button) override;
    
    private:
        HostAudioProcessorPlayer& hostAudioProcessorPlayer;
    
        void sliderValueChanged (Slider* slider) override;
        PlaybackLookAndFeel playbackLookAndFeel;
    
        DrawableButton playButton;
        DrawableButton stopButton;
        Slider bpmSlider;
    
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaybackComponent)
};
