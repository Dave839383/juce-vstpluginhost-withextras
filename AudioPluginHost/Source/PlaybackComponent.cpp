/*
  ==============================================================================

    PlaybackComponent.cpp
    Created: 18 Mar 2018 6:27:04pm
    Author:  David Lloyd

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "PlaybackComponent.h"

PlaybackLookAndFeel::PlaybackLookAndFeel()
{
    setColour (Slider::thumbColourId, Colours::transparentBlack);
    setColour (Slider::trackColourId, Colours::transparentBlack);
    setColour (Slider::backgroundColourId, Colours::transparentBlack);
}

void PlaybackLookAndFeel::drawLabel (Graphics& g, Label& label)
{
    g.fillAll (label.findColour (Label::backgroundColourId));
    
    if (! label.isBeingEdited())
    {
        const float alpha = label.isEnabled() ? 1.0f : 0.5f;
        Font font (getLabelFont (label));
        font.setHeight (14.0);
        g.setColour (Colours::black.withMultipliedAlpha (alpha));
        g.setFont (font);
        
        Rectangle<int> textArea (label.getBorderSize().subtractedFrom (label.getLocalBounds()));
        
        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());
        
        g.setColour (Colours::transparentBlack);
    }
    else if (label.isEnabled())
    {
        g.setColour (Colours::transparentBlack);
    }
    
    g.drawRect (label.getLocalBounds());
}

PlaybackComponent::PlaybackComponent(HostAudioProcessorPlayer& hpp) : hostAudioProcessorPlayer (hpp), playButton("Play Button", DrawableButton::ImageOnButtonBackground), stopButton("Stop Button", DrawableButton::ImageOnButtonBackground)
{
    playButton.setColour(TextButton::buttonColourId, Colours::black);
    addAndMakeVisible (playButton);
    playButton.addListener (this);
    
    stopButton.setColour(TextButton::buttonColourId, Colours::black);
    addAndMakeVisible (stopButton);
    stopButton.addListener (this);
    
    bpmSlider.setSliderStyle(Slider::LinearBarVertical);
    bpmSlider.setVelocityBasedMode(true);
    bpmSlider.setVelocityModeParameters(5.0);
    bpmSlider.setRange (1, 300, 1);
    bpmSlider.setValue(120);
    bpmSlider.addListener(this);
    bpmSlider.setLookAndFeel(&playbackLookAndFeel);
    bpmSlider.setTextValueSuffix (" bpm");
    addAndMakeVisible (bpmSlider);
    
    // setSize calls resized in this class.
    setSize(300, 200);
}

void PlaybackComponent::paint (Graphics& g)
{
    g.setColour(Colour::fromRGB(203, 203, 203));
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 5);
}

void PlaybackComponent::resized()
{
    auto border = 2;
    
    Rectangle<int> area = getLocalBounds();
    
    int buttonArea = area.getWidth() / 4;
    
    // create Play Button Triangle
    DrawablePath playDrawablePath, stopDrawablePath;
    
    Path playPath;
    playPath.addTriangle(0, 0, 10, 5, 0, 10);
    playDrawablePath.setPath(playPath);
    FillType f(Colours::green);
    playDrawablePath.setFill(f);
    
    playButton.setImages(&playDrawablePath);
    playButton.setEdgeIndent(2);
    
    Path stopPath;
    stopPath.addRectangle(0, 0, 10, 10);
    stopDrawablePath.setPath(stopPath);
    FillType fs(Colours::red);
    stopDrawablePath.setFill(fs);
    
    stopButton.setImages(&stopDrawablePath);
    stopButton.setEdgeIndent(2);

    playButton.setBounds (area.removeFromLeft (buttonArea).reduced (border));
    stopButton.setBounds (area.removeFromLeft (buttonArea).reduced (border));
    bpmSlider.setBounds (area.removeFromLeft (buttonArea * 2).reduced (border));
}

void PlaybackComponent::buttonClicked (Button* button)
{
     if (button == &playButton && !hostAudioProcessorPlayer.getIsPlaying())
     {
         hostAudioProcessorPlayer.setIsPlaying(true);
     }
     else if (button == &stopButton && hostAudioProcessorPlayer.getIsPlaying())
     {
         hostAudioProcessorPlayer.setIsPlaying(false);
     }
}

void PlaybackComponent::sliderValueChanged (Slider* slider)
{
    if (slider == &bpmSlider)
    {
        hostAudioProcessorPlayer.setBpm(bpmSlider.getValue());
    }
}
