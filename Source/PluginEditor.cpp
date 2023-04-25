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
      tabbedComponent->addTab(juce::String(i+1), backgroundColor, dexedComponents[i].get(), true);
      dexedEditor = pluginAudioProcessor->dexedPluginInstances[i]->getActiveEditor();
      dexedComponents[i]->setSize(dexedEditor->getWidth(), dexedEditor->getHeight());
      tabbedComponent->setSize(dexedComponents[i]->getWidth(), dexedComponents[i]->getHeight() + tabbedComponent->getTabBarDepth());
    }
        
    // Make the tabbed component visible
    tabbedComponent->setVisible(true);

    // Set the size of the editor window
    setSize(tabbedComponent->getWidth(), tabbedComponent->getHeight());

}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    delete dexedEditor;
}

//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{

}

void PluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
