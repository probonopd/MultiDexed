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
    auto* pluginAudioProcessor = &audioProcessor;

    // Create a tabbed component
    tabbedComponent = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(*tabbedComponent);

    // Get the background color of the window (used for tab background)
    auto backgroundColor = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);

    // Create a tab for each instance of Dexed
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; i++) {
        dexedComponents[i] = std::make_unique<juce::Component>(); // Create container for this tab

        // Ensure the editor exists or create it
        pluginAudioProcessor->dexedPluginInstances[i]->createEditorIfNeeded();
        auto* activeEditor = pluginAudioProcessor->dexedPluginInstances[i]->getActiveEditor();

        if (activeEditor) {
            dexedEditors[i] = activeEditor; // Store the pointer

            // ---> FIX ATTEMPT: Explicitly remove from any potential old parent <---
            // if (auto* oldParent = activeEditor->getParentComponent()) {
            //     oldParent->removeChildComponent(activeEditor);
            // }
            // ---> END FIX ATTEMPT <---

            // Add the Dexed editor to our new container component for this tab
            dexedComponents[i]->addAndMakeVisible(activeEditor);

            // Size the container component to match the Dexed editor's size.
            int editorWidth = activeEditor->getWidth() > 0 ? activeEditor->getWidth() : 600;
            int editorHeight = activeEditor->getHeight() > 0 ? activeEditor->getHeight() : 400;
            dexedComponents[i]->setSize(editorWidth, editorHeight);

            // Ensure the Dexed editor itself fills its container.
            activeEditor->setBounds(0, 0, editorWidth, editorHeight);

            // Add the container component (which holds the Dexed editor) as a tab page
            juce::String tabName = (i == 0) ? "Master" : juce::String("Dexed ") + juce::String(i);
            tabbedComponent->addTab(tabName, backgroundColor, dexedComponents[i].get(), true);

        } else {
            DBG("Error: Could not get active editor for Dexed instance " + juce::String(i));
            dexedEditors[i] = nullptr;
            juce::String tabName = (i == 0) ? "Master (Error)" : juce::String("Dexed ") + juce::String(i) + " (Error)";
            tabbedComponent->addTab(tabName, juce::Colours::red, nullptr, true);
        }
    } // --- END OF THE INSTANCE LOOP ---

    // --- Determine Overall Size (Moved Outside Loop) ---
    int finalTabContentWidth = 600;
    int finalTabContentHeight = 400;
    int tabBarDepth = tabbedComponent->getTabBarDepth();

    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; ++i) {
        if (dexedEditors[i] != nullptr && dexedEditors[i]->getWidth() > 0 && dexedEditors[i]->getHeight() > 0) {
            finalTabContentWidth = dexedEditors[i]->getWidth();
            finalTabContentHeight = dexedEditors[i]->getHeight();
            break;
        }
    }

    tabbedComponent->setSize(finalTabContentWidth, finalTabContentHeight + tabBarDepth);

    const int controlsAreaHeight = 100;
    setSize(tabbedComponent->getWidth(), tabbedComponent->getHeight() + controlsAreaHeight);
    // --- End Sizing Logic ---


    // Sliders for the MultiDexed parameters
    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    detuneSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "detuneSpread", detuneSlider);
    addAndMakeVisible(detuneLabel);
    detuneLabel.setText("Detune", juce::dontSendNotification);
    detuneLabel.attachToComponent(&detuneSlider, false);
    detuneLabel.setJustificationType(juce::Justification::centred);


    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "panSpread", panSlider);
    addAndMakeVisible(panLabel);
    panLabel.setText("Pan Spread", juce::dontSendNotification);
    panLabel.attachToComponent(&panSlider, false);
    panLabel.setJustificationType(juce::Justification::centred);

}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    detuneSliderAttachment = nullptr;
    panSliderAttachment = nullptr;

    for (int i = 0; i < audioProcessor.numberOfInstances; i++) {
        dexedEditors[i] = nullptr;
    }

    tabbedComponent = nullptr;
}

//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto controlsArea = bounds.removeFromTop(100);

    int sliderWidth = 80;
    int sliderHeight = 80;
    // int labelHeight = 20; // Not needed if using attachToComponent

    juce::Rectangle<int> detuneSliderArea = controlsArea.removeFromLeft(sliderWidth + 20);
    detuneSlider.setBounds(detuneSliderArea.getX() + 10, detuneSliderArea.getY(), sliderWidth, sliderHeight);

    juce::Rectangle<int> panSliderArea = controlsArea.removeFromLeft(sliderWidth + 20);
    panSlider.setBounds(panSliderArea.getX() + 10, panSliderArea.getY(), sliderWidth, sliderHeight);

    // Position the tabbed component in the remaining area
    tabbedComponent->setBounds(bounds);

    // --- FIX for Build Error C2039 ---
    // Resize the *content* components within each tab
    for (int i = 0; i < tabbedComponent->getNumTabs(); ++i) {
        if (auto* tabContentComponent = tabbedComponent->getTabContentComponent(i)) {
             // Calculate the bounds available for the tab's content
             auto contentBounds = tabbedComponent->getLocalBounds(); // Start with the TabbedComponent's full area
             // Remove the area occupied by the tab bar (assuming tabs are at the top)
             contentBounds.removeFromTop(tabbedComponent->getTabBarDepth());

             // Set the bounds of our container component (e.g., dexedComponents[i])
             tabContentComponent->setBounds(contentBounds);

             // Now ensure the Dexed editor *inside* the container is also resized
             // to fill its container (which now has the correct contentBounds)
             if (tabContentComponent->getNumChildComponents() > 0) {
                 if(auto* dexedEditorComp = tabContentComponent->getChildComponent(0)) {
                     dexedEditorComp->setBounds(tabContentComponent->getLocalBounds()); // Fill the container
                 }
             }
        }
    }
    // --- End Fix ---
}
