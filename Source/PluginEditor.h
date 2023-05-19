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
class PluginAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    PluginAudioProcessorEditor (PluginAudioProcessor&);
    ~PluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginAudioProcessor& audioProcessor;

    // Pointer to our button
    juce::TextButton* button;

    // Pointer to our tabbed component
    std::unique_ptr<juce::TabbedComponent> tabbedComponent;

    // Array with 8 pointers to our Dexed components
    std::unique_ptr<juce::Component> dexedComponents[8];

    // Array with 8 pointers to our Dexed editors
    juce::AudioProcessorEditor* dexedEditors[8];

    // Sliders for the MultiDexed parameters
    juce::Slider detuneSlider;
    juce::Slider panSlider;

    // Labels for the sliders
    juce::Label detuneLabel;
    juce::Label panLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorEditor)
};
