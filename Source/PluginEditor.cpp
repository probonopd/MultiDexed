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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() { }

//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{

    // Get our PluginAudioProcessor instance that is defined in PluginProcessor.h
    auto pluginAudioProcessor = dynamic_cast<PluginAudioProcessor *>(getAudioProcessor());

    // Show its user interface (editor)
    auto editor = pluginAudioProcessor->dexedPluginInstance1->createEditorIfNeeded();

    // Ask the plugin for the width and height of its editor
    auto width = editor->getWidth();
    auto height = editor->getHeight();

    // Set the size of our editor to match the plugin's editor
    setSize(width, height);

    // Make the plugin's editor visibles
    editor->setVisible(true);
    addAndMakeVisible(editor);
}

void PluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
