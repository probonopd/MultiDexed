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
// Make the editor listen to tab changes
class PluginAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    public juce::TabbedComponent::Listener // Add this inheritance
{
public:
    PluginAudioProcessorEditor (PluginAudioProcessor&);
    ~PluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // --- Add TabbedComponent Listener Method ---
    void currentTabChanged (int newCurrentTabIndex, const juce::String& newCurrentTabName) override;
    // ---

private:
    // This reference is provided as a quick way for the editor to
    // access the processor object that created it.
    PluginAudioProcessor& audioProcessor;

    // Pointer to our tabbed component
    std::unique_ptr<juce::TabbedComponent> tabbedComponent;

    // Array with 8 pointers to our Dexed container components
    std::unique_ptr<juce::Component> dexedComponents[8];

    // Array with 8 pointers to our Dexed editors - We might not need this array anymore
    // if we fetch the editor dynamically in currentTabChanged. Keep it for now if needed elsewhere.
    juce::AudioProcessorEditor* dexedEditors[8] = { nullptr }; // Initialize to null

    // Sliders for the MultiDexed parameters
    juce::Slider detuneSlider;
    juce::Slider panSlider;

    // Attach the sliders to the parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panSliderAttachment;

    // Labels for the sliders
    juce::Label detuneLabel; // Needs to be added/managed in .cpp
    juce::Label panLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorEditor)
};
