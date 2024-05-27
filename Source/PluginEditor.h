/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

//==============================================================================
/**
*/
class BasicCompressorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BasicCompressorAudioProcessorEditor (BasicCompressorAudioProcessor&,juce::AudioProcessorValueTreeState&);
    ~BasicCompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void addSlider(juce::String name, juce::String labelText, juce::Slider& slider,
                   juce::Label& label, std::unique_ptr<SliderAttachment>& attachment);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BasicCompressorAudioProcessor& audioProcessor;
    
    juce::AudioProcessorValueTreeState& params;
    
    juce::Slider threshSlider,slopeSlider,kneeSlider,attackSlider,releaseSlider;
    
    juce::Label threshLabel, slopeLabel, kneeLabel, attackLabel, releaseLabel;
    
    std::unique_ptr<SliderAttachment> threshAttachment, slopeAttachment, kneeAttachment, attackAttachment,releaseAttachment;
    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicCompressorAudioProcessorEditor)
};
