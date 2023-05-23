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
class PluginAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    PluginAudioProcessorEditor (PluginAudioProcessor&);
    ~PluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginAudioProcessor& audioProcessor;

    // Pointer to our button
    juce::TextButton button;

    // Pointer to our tabbed component
    std::unique_ptr<juce::TabbedComponent> tabbedComponent;

    // Array with 8 pointers to our Dexed windows
    std::unique_ptr<juce::DocumentWindow> dexedWindows[8];

    // Array with 8 pointers to our Dexed editors
    juce::AudioProcessorEditor* dexedEditors[8];

    // Sliders for the MultiDexed parameters
    juce::Slider detuneSlider;
    juce::Slider panSlider;

    // Attach the sliders to the parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panSliderAttachment;
    
    // Labels for the sliders
    juce::Label detuneLabel;
    juce::Label panLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorEditor)
};
