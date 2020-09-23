/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VstAudioPlayerAudioProcessorEditor::VstAudioPlayerAudioProcessorEditor (VstAudioPlayerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    thumbnail = new AudioThumbnailComp(processor.formatManager, processor.transportSource, processor.thumbnailCache, processor.currentlyLoadedFile);
    addAndMakeVisible(thumbnail);
    thumbnail->addChangeListener(this);

    addAndMakeVisible(startStopButton);
    startStopButton.setButtonText("Play/Stop");
    startStopButton.addListener(this);
    startStopButton.setColour(TextButton::buttonColourId, Colour(0xff79ed7f));

    addAndMakeVisible(openButton);
    openButton.setButtonText("Open");
    openButton.addListener(this);
    openButton.setColour(TextButton::buttonColourId, Colour(0xff79ed7f));

    setOpaque(true);

    setSize(512, 220);
}

VstAudioPlayerAudioProcessorEditor::~VstAudioPlayerAudioProcessorEditor()
{
    thumbnail->removeChangeListener(this);
}

//==============================================================================
void VstAudioPlayerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::grey);
}

void VstAudioPlayerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    Rectangle<int> r(getLocalBounds().reduced(4));

    Rectangle<int> controls(r.removeFromBottom(32));

    startStopButton.setBounds(controls);
    openButton.setBounds(controls);

    r.removeFromBottom(6);
    thumbnail->setBounds(r.removeFromBottom(180));
    r.removeFromBottom(6);
}

void VstAudioPlayerAudioProcessorEditor::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &startStopButton)
    {
        if (processor.transportSource.isPlaying())
        {
            processor.transportSource.stop();
        }
        else
        {
            processor.transportSource.setPosition(0);
            processor.transportSource.start();
        }
    }
    if (buttonThatWasClicked == &openButton)
    {
        if (processor.transportSource.isPlaying())
        {
            processor.transportSource.stop();
        }
        processor.transportSource.start();
        juce::FileChooser chooser("Select a Wave file to play...",
            {},
            "*.wav");
        if (chooser.browseForFileToOpen())                                          // [8]
        {
            auto file = chooser.getResult();                                        // [9]
            processor.loadFileIntoTransport(file);
        }
    }
}

void VstAudioPlayerAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == thumbnail)
    {
        processor.loadFileIntoTransport(thumbnail->getLastDroppedFile());
        thumbnail->setFile(thumbnail->getLastDroppedFile());
    }
}