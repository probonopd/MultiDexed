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

    // Return if the pluginAudioProcessor is null
    if (pluginAudioProcessor == nullptr)
        return;

    // Add the Dexed editors to the window
    dexedEditors[0] = pluginAudioProcessor->dexedPluginInstances[0]->createEditorIfNeeded();
    dexedEditors[0]->setSize(dexedEditors[0]->getWidth(), dexedEditors[0]->getHeight());
    addAndMakeVisible(dexedEditors[0]);
    setSize(dexedEditors[0]->getWidth(), dexedEditors[0]->getHeight()+100);

    // Move down by 100 pixels
    dexedEditors[0]->setTopLeftPosition(0, 100);

    // Sliders for the MultiDexed parameters

    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    detuneSlider.setRange(pluginAudioProcessor->detuneSpread->getNormalisableRange().start,
                          pluginAudioProcessor->detuneSpread->getNormalisableRange().end,
                          pluginAudioProcessor->detuneSpread->getNormalisableRange().interval);
    detuneSlider.setValue(pluginAudioProcessor->detuneSpread->getCurrentValueAsText().getFloatValue());
    detuneSlider.addListener(this);
    addAndMakeVisible(detuneLabel);
    detuneLabel.setText("Detune", juce::dontSendNotification);
    detuneLabel.attachToComponent(&detuneSlider, false);

    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    panSlider.setRange(pluginAudioProcessor->panSpread->getNormalisableRange().start,
                       pluginAudioProcessor->panSpread->getNormalisableRange().end,
                       pluginAudioProcessor->panSpread->getNormalisableRange().interval);
    panSlider.setValue(pluginAudioProcessor->panSpread->getCurrentValueAsText().getFloatValue());
    panSlider.addListener(this);
    addAndMakeVisible(panLabel);
    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.attachToComponent(&panSlider, false);
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

    panSlider.setBounds(0, 0, 100, 100);
    detuneSlider.setBounds(100, 0, 100, 100);




}

void PluginAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    // Get our PluginAudioProcessor instance that is defined in PluginProcessor.h
    auto pluginAudioProcessor = dynamic_cast<PluginAudioProcessor *>(getAudioProcessor());

    // Get the value of the slider
    float sliderValue = slider->getValue();

    // Set the value of the parameter
    if (slider == &detuneSlider) {
        pluginAudioProcessor->detuneSpread->setValueNotifyingHost(sliderValue);
    }
    else if (slider == &panSlider) {
        pluginAudioProcessor->panSpread->setValueNotifyingHost(sliderValue);
    }
}