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
    auto pluginAudioProcessor = dynamic_cast<PluginAudioProcessor *>(getAudioProcessor());

    // Create a tabbed component
    tabbedComponent = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(*tabbedComponent);

    // Get the background color of the window
    auto backgroundColor = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);

    // Create a tab for each instance of Dexed
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; i++) {
        dexedComponents[i] = std::make_unique<juce::Component>();
        pluginAudioProcessor->dexedPluginInstances[i]->createEditorIfNeeded();
        dexedComponents[i]->addAndMakeVisible(pluginAudioProcessor->dexedPluginInstances[i]->getActiveEditor());
        // Name the first tab "Master", and the rest "Dexed 1", "Dexed 2", etc.
        if (i == 0) {
            tabbedComponent->addTab(juce::String("Master"), backgroundColor, dexedComponents[i].get(), true);
        }
        else {
            tabbedComponent->addTab(juce::String("Dexed ") + juce::String(i), backgroundColor, dexedComponents[i].get(), true);
        }
        dexedEditors[i] = pluginAudioProcessor->dexedPluginInstances[i]->getActiveEditor();
        dexedComponents[i]->setSize(dexedEditors[i]->getWidth(), dexedEditors[i]->getHeight());
        tabbedComponent->setSize(dexedComponents[i]->getWidth(), dexedComponents[i]->getHeight() + tabbedComponent->getTabBarDepth());
    }
        
    // Make the tabbed component visible
    tabbedComponent->setVisible(true);

    // Set the size of the editor window
    setSize(tabbedComponent->getWidth(), tabbedComponent->getHeight() + 100);

    // Sliders for the MultiDexed parameters
    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);

    detuneSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "detuneSpread", detuneSlider);

    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "panSpread", panSlider);
    addAndMakeVisible(panLabel);
    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.attachToComponent(&panSlider, false);

    // Repaint the tabbed component
    tabbedComponent->repaint();

    // Repaint the contents of the first tab
    if (tabbedComponent->getNumTabs() > 0)
    {
        juce::Component* firstTabContent = tabbedComponent->getTabContentComponent(0);
        if (firstTabContent != nullptr)
        {
            firstTabContent->setVisible(true);
            firstTabContent->repaint();
        }
    }

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
