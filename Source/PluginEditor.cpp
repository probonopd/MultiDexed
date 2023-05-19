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

    // Add the Dexed editors to the window
    dexedEditors[0] = pluginAudioProcessor->dexedPluginInstances[0]->createEditorIfNeeded();
    dexedEditors[0]->setSize(dexedEditors[0]->getWidth(), dexedEditors[0]->getHeight());
    addAndMakeVisible(dexedEditors[0]);
    dexedEditors[0]->setVisible(true);
    dexedEditors[0]->setTopLeftPosition(0, 0);
    setSize(dexedEditors[0]->getWidth(), dexedEditors[0]->getHeight());
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    // Get our PluginAudioProcessor instance that is defined in PluginProcessor.h
    auto pluginAudioProcessor = dynamic_cast<PluginAudioProcessor *>(getAudioProcessor());
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; i++) {
      dexedEditors[i] = nullptr;
      dexedComponents[i] = nullptr;
    }
    tabbedComponent = nullptr;
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
