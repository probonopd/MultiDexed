/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PluginAudioProcessorEditor (PluginAudioProcessor&);
    ~PluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginAudioProcessor& audioProcessor;

    // Pointer to Dexed's editor instance
    juce::AudioProcessorEditor* dexedEditor;

    // Pointer to our button
    juce::TextButton* button;

    // Pointer to our tabbed component
    std::unique_ptr<juce::TabbedComponent> tabbedComponent;

    // Array with 8 pointers to our Dexed components
    std::unique_ptr<juce::Component> dexedComponents[8];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorEditor)
};
