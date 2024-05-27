/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicCompressorAudioProcessor::BasicCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
state(*this, nullptr, juce::Identifier("params"), {
    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("thresh",1),"Threshold",
                                                juce::NormalisableRange<float>(-60.0f,20.0f,0.01f),10.0f),
    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ratio",1),"Ratio",
                                                juce::NormalisableRange<float>(1.0f,20.0f,0.01f),0.0f),
    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("knee",1),"KneeWidth",
                                                juce::NormalisableRange<float>(0.0f,24.0f,0.01f),0.0f),
    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack",1),"Attack",
                                                juce::NormalisableRange<float>(0.01f,500.0f,0.01f),100.f),
    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release",1),"Release",
                                                juce::NormalisableRange<float>(0.01f,2000.f,0.01f),500.f)
    
})
#endif
{
    
}

BasicCompressorAudioProcessor::~BasicCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String BasicCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    for (int channel = 0; channel<getTotalNumOutputChannels(); channel++)
        allCompressors.add(Compressor());
    
    threshParam = state.getRawParameterValue("thresh");
    slopeParam = state.getRawParameterValue("ratio");
    kneeParam = state.getRawParameterValue("knee");
    attackParam = state.getRawParameterValue("attack");
    releaseParam = state.getRawParameterValue("release");
}

void BasicCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void BasicCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    float at = 1-std::pow(juce::MathConstants<float>::euler, ((1/getSampleRate())*-2.2f)/(*attackParam/1000.0f));
    float rt = 1-std::pow(juce::MathConstants<float>::euler, ((1/getSampleRate())*-2.2f)/(*releaseParam/1000.0f));
    
    for (int i = 0; i<buffer.getNumSamples();++i)
    {
        for (int channel = 0; channel<getTotalNumOutputChannels(); channel++)
        {
            auto* data = buffer.getWritePointer(channel);
            Compressor* comp  = &allCompressors.getReference(channel);
            data[i]= comp->compressSample(data[i], *threshParam,*slopeParam,at,rt,*kneeParam);
        }
    }
}

//==============================================================================
bool BasicCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicCompressorAudioProcessor::createEditor()
{
    return new BasicCompressorAudioProcessorEditor (*this,state);
}

//==============================================================================
void BasicCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto stateTree = state.copyState();
    std::unique_ptr<XmlElement> xml(stateTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void BasicCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if(xmlState.get() != nullptr && xmlState->hasTagName(state.state.getType()))
    {
        state.replaceState(ValueTree::fromXml(*xmlState));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicCompressorAudioProcessor();
}
