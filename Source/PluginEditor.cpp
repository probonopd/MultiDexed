/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginAudioProcessorEditor::PluginAudioProcessorEditor(PluginAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Get our PluginAudioProcessor instance that is defined in PluginProcessor.h
    auto pluginAudioProcessor = &audioProcessor; // Or dynamic_cast<PluginAudioProcessor *>(getAudioProcessor());

    // Create a tabbed component
    tabbedComponent = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(*tabbedComponent);

    // Get the background color of the window
    auto backgroundColor = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);

    // Create a tab for each instance of Dexed
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; i++) {

        dexedComponents[i] = std::make_unique<juce::Component>(); // Create the container component

        // Ensure the editor exists (it should, but good practice)
        pluginAudioProcessor->dexedPluginInstances[i]->createEditorIfNeeded();
        auto* activeEditor = pluginAudioProcessor->dexedPluginInstances[i]->getActiveEditor();

        if (activeEditor) // Check if we got a valid editor pointer
        {
            dexedEditors[i] = activeEditor; // Store the editor pointer
            dexedComponents[i]->addAndMakeVisible(activeEditor); // Add Dexed's editor to our container

            // Set the size of the *container* component to match the Dexed editor's size
            // This ensures the container clips or holds the Dexed editor correctly.
            dexedComponents[i]->setSize(activeEditor->getWidth(), activeEditor->getHeight());

            activeEditor->repaint();        // Ask the Dexed editor itself to redraw
            dexedComponents[i]->repaint();  // Ask our container component to redraw

            // Add the container component (which now holds the Dexed editor) to the tab
            juce::String tabName = (i == 0) ? "Master" : juce::String("Dexed ") + juce::String(i);
            tabbedComponent->addTab(tabName, backgroundColor, dexedComponents[i].get(), true);

            // Adjust overall tabbed component size based on the *first* editor found
            if (i == 0) { // Or maybe track the largest size found so far
                 tabbedComponent->setSize(activeEditor->getWidth(), activeEditor->getHeight() + tabbedComponent->getTabBarDepth());
            }
        }
        else
        {
            // Handle the error case where an editor couldn't be created or retrieved
            DBG("Error: Could not get active editor for Dexed instance " + juce::String(i));
            dexedEditors[i] = nullptr;
            juce::String tabName = (i == 0) ? "Master (Error)" : juce::String("Dexed ") + juce::String(i) + " (Error)";
             tabbedComponent->addTab(tabName, juce::Colours::red, nullptr, true); // Add tab with no component
        }
    }

    // Set the initial size of the main editor window
    // Ensure tabbedComponent has a valid size before using getWidth/getHeight
    if(tabbedComponent->getNumTabs() > 0 && dexedComponents[0] != nullptr) {
         setSize(tabbedComponent->getWidth(), tabbedComponent->getHeight() + 100); // +100 for sliders
    } else {
         setSize(600, 500); // Default fallback size
    }

    // Sliders for the MultiDexed parameters
    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    detuneSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "detuneSpread", detuneSlider);
    addAndMakeVisible(detuneLabel);
    detuneLabel.setText("Detune", juce::dontSendNotification);
    detuneLabel.attachToComponent(&detuneSlider, false); // Attach below

    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "panSpread", panSlider);
    addAndMakeVisible(panLabel);
    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.attachToComponent(&panSlider, false); // Attach below
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    // Clean up Dexed components and detach slider attachments
    for (int i = 0; i < audioProcessor.numberOfInstances; i++) {
        dexedEditors[i] = nullptr;
        dexedComponents[i] = nullptr;
    }

    tabbedComponent = nullptr;
    detuneSliderAttachment = nullptr;
    panSliderAttachment = nullptr;
}

//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{

}

void PluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    panSlider.setBounds(0, 0, 100, 100);
    detuneSlider.setBounds(100, 0, 100, 100);


    // Add tabbed component to hold the Dexed editors
    tabbedComponent->setBounds(0, 100, getWidth(), getHeight() - 100);
}
