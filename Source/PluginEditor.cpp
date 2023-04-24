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

    // Show its user interface (editor)
    dexedEditor = pluginAudioProcessor->dexedPluginInstance1->createEditorIfNeeded();

    // Ask the plugin for the width and height of its editor
    auto width = dexedEditor->getWidth();
    auto height = dexedEditor->getHeight();

    // Set the size of our editor to match the plugin's editor
    setSize(width, height);

    // Make the plugin's editor visibles
    dexedEditor->setVisible(true);
    addAndMakeVisible(dexedEditor);
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
