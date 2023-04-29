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
    float heightFactor = 0.86;
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; i++) {
      dexedEditors[i] = pluginAudioProcessor->dexedPluginInstances[i]->createEditorIfNeeded();
      // Scale the user interface of the Dexed editor to 0.5
      dexedEditors[i]->setScaleFactor(0.5);
      // Crop the Dexed editor: reduce the height to cut off the virtual keyboard
      dexedEditors[i]->setSize(dexedEditors[i]->getWidth(), dexedEditors[i]->getHeight() * heightFactor);
      addAndMakeVisible(dexedEditors[i]);
      // Show the Dexed editor
      dexedEditors[i]->setVisible(true);
      // Move each Dexed editor: 3 rows of up to 3 editors
      dexedEditors[i]->setTopLeftPosition((i % 3) * dexedEditors[i]->getWidth(), (i / 3) * dexedEditors[i]->getHeight());
    }

    // Set size of the window to the size of the Dexed editors
    setSize(dexedEditors[0]->getWidth() * 3, dexedEditors[0]->getHeight() * 3);

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
