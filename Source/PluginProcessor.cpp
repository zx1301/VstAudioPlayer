/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VstAudioPlayerAudioProcessor::VstAudioPlayerAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
                       thumbnailCache(1),
                       readAheadThread("transport read ahead")
{
    formatManager.registerBasicFormats();
    readAheadThread.startThread(3);
}


VstAudioPlayerAudioProcessor::~VstAudioPlayerAudioProcessor()
{
    transportSource.setSource(nullptr);
}

//==============================================================================
const juce::String VstAudioPlayerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VstAudioPlayerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VstAudioPlayerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VstAudioPlayerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VstAudioPlayerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VstAudioPlayerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VstAudioPlayerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VstAudioPlayerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VstAudioPlayerAudioProcessor::getProgramName (int index)
{
    return {};
}

void VstAudioPlayerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VstAudioPlayerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void VstAudioPlayerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    transportSource.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VstAudioPlayerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VstAudioPlayerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    transportSource.getNextAudioBlock(AudioSourceChannelInfo(buffer));
}

//==============================================================================
bool VstAudioPlayerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VstAudioPlayerAudioProcessor::createEditor()
{
    return new VstAudioPlayerAudioProcessorEditor (*this);
}

//==============================================================================
void VstAudioPlayerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement xml("plugin-settings");

    xml.setAttribute("audioFile", currentlyLoadedFile.getFullPathName());

    copyXmlToBinary(xml, destData);
}

void VstAudioPlayerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName("plugin-settings"))
        {
            currentlyLoadedFile = File::createFileWithoutCheckingPath(xmlState->getStringAttribute("audioFile"));
            if (currentlyLoadedFile.existsAsFile())
            {
                loadFileIntoTransport(currentlyLoadedFile);
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VstAudioPlayerAudioProcessor();
}

void VstAudioPlayerAudioProcessor::loadFileIntoTransport (const File& audioFile)
{
    // unload the previous file source and delete it..
    transportSource.stop();
    transportSource.setSource(nullptr);
    currentAudioFileSource = nullptr;
    
    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
    currentlyLoadedFile = audioFile;

    if (reader != nullptr)
    {
        currentAudioFileSource = new AudioFormatReaderSource(reader, true);

        // ..and plug it into our transport source
        transportSource.setSource(
            currentAudioFileSource,
            32768,                   // tells it to buffer this many samples ahead
            &readAheadThread,        // this is the background thread to use for reading-ahead
            reader->sampleRate);     // allows for sample rate correction
    }
    
    /*
    juce::FileChooser chooser("Select a Wave file to play...",
        {},
        "*.wav");                                        // [7]

    if (chooser.browseForFileToOpen())                                          // [8]
    {
        auto file = chooser.getResult();                                        // [9]
        auto* reader = formatManager.createReaderFor(file);                    // [10]

        if (reader != nullptr)
        {
            currentAudioFileSource = new AudioFormatReaderSource(reader, true);

            // ..and plug it into our transport source
            transportSource.setSource(
                currentAudioFileSource,
                32768,                   // tells it to buffer this many samples ahead
                &readAheadThread,        // this is the background thread to use for reading-ahead
                reader->sampleRate);     // allows for sample rate correction
        }
    }
    */
}
