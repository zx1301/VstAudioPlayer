/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioThumbnailComp.h"
#include "PluginProcessor.h"

using namespace juce;
//==============================================================================
/**
*/
class VstAudioPlayerAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::ChangeListener,
    private juce::Button::Listener
{
public:
    VstAudioPlayerAudioProcessorEditor (VstAudioPlayerAudioProcessor&);
    ~VstAudioPlayerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VstAudioPlayerAudioProcessor& processor;

    ScopedPointer<AudioThumbnailComp> thumbnail;

    TextButton startStopButton;
    TextButton openButton;

    void buttonClicked(Button* buttonThatWasClicked) override;

    void changeListenerCallback(ChangeBroadcaster* source) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VstAudioPlayerAudioProcessorEditor)
};
