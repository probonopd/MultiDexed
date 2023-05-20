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


    setSize(800, 600);

    // Sliders for the MultiDexed parameters
    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    detuneSlider.setRange(pluginAudioProcessor->detuneSpread->getNormalisableRange().start,
                          pluginAudioProcessor->detuneSpread->getNormalisableRange().end,
                          pluginAudioProcessor->detuneSpread->getNormalisableRange().interval);
    detuneSlider.setValue(pluginAudioProcessor->detuneSpread->getCurrentValueAsText().getFloatValue());
    detuneSlider.addListener(this);


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
    DBG("Pan slider value: " << panSlider.getValue());

    // Initialize button
    button.setButtonText("Open Dexed");
    addAndMakeVisible(button);
    button.addListener(this);
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    // Get our PluginAudioProcessor instance that is defined in PluginProcessor.h
    auto pluginAudioProcessor = dynamic_cast<PluginAudioProcessor *>(getAudioProcessor());
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; i++) {
      dexedEditors[i] = nullptr;
      dexedWindows[i] = nullptr;
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

    // Add push button to open Dexed editor for instance 0
    button.setBounds(200, 0, 100, 100);

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

void PluginAudioProcessorEditor::buttonClicked(juce::Button *button)
{
    // Print a message to the console when the button is clicked
    DBG("Button clicked!");

    // If the window is already open, don't open another one but bring it to the front
    if (dexedWindows[0] != nullptr) {
        dexedWindows[0]->toFront(true);
        return;
    }
    
    // Make a window to hold the Dexed editor
    const auto bg = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).darker();
    auto component = std::make_unique<juce::Component>();
    auto window = std::make_unique<juce::DocumentWindow> ("Dexed", bg, juce::DocumentWindow::allButtons);

    // Get our PluginAudioProcessor instance that is defined in PluginProcessor.h
    auto pluginAudioProcessor = dynamic_cast<PluginAudioProcessor *>(getAudioProcessor());

    // Add Dexed editor for instance 0 to the window
    dexedEditors[0] = pluginAudioProcessor->dexedPluginInstances[0]->createEditor();
    component->addAndMakeVisible(dexedEditors[0]);
    window->setContentOwned(component.release(), true);
    window->setUsingNativeTitleBar(true);
    window->setSize(dexedEditors[0]->getWidth(), dexedEditors[0]->getHeight());
    window->setAlwaysOnTop (false);
    window->centreAroundComponent (this, window->getWidth(), window->getHeight());
    window->setVisible (true);
    window->setResizable (false, false);

    // Make it so that the window doesn't disappear when this function ends
    // Possibly this is wrong. FIXME: How to do this properly?
    dexedWindows[0] = std::move(window); // Move ownership of the unique_ptr to dexedComponents[0]

    // TODO: Make the close button on the Dexed editor close the window
    // FIXME: How to do this? Not even the HostPluginDemo from JUCE 7 does this
    // All we get when the close button is clicked is a message saying:
    // "JUCE Assertion failure in juce_DocumentWindow.cpp:173"
    // What does this mean and what do we do about it?

    // FIXME: Why does this crash in REAPER when trying to load a different .syx cartridge?
    // In the standalone app, it works fine. In REAPER on Linux, it stops responding
    // as soon as one clicks the "LOAD" button in the Dexed editor

    // FIXME: Why does using the sliders in the main window after having opened
    // the Dexed window crash REAPER on Linux? It works in the standalone version.
}