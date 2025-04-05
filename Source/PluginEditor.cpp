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
        // --- Start of Added Block ---
        auto* activeEditor = pluginAudioProcessor->dexedPluginInstances[i]->getActiveEditor();

        if (activeEditor) // Check if we got a valid editor pointer
        {
            // ---> FIX ATTEMPT: Explicitly remove from any potential old parent <---
            // If the editor still thinks it has a parent from the *previous* window instance,
            // this might help detach it cleanly before adding it to the new hierarchy.
            if (auto* oldParent = activeEditor->getParentComponent()) {
                oldParent->removeChildComponent(activeEditor);
            }
            // ---> END FIX ATTEMPT <---

            dexedComponents[i]->addAndMakeVisible(activeEditor); // Add Dexed's editor to our container
        }
        // --- End of Added Block (Original line replaced by the block above) ---

        // Name the first tab "Master", and the rest "Dexed 1", "Dexed 2", etc.
        if (i == 0) {
            tabbedComponent->addTab(juce::String("Master"), backgroundColor, dexedComponents[i].get(), true);
        }
        else {
            tabbedComponent->addTab(juce::String("Dexed ") + juce::String(i), backgroundColor, dexedComponents[i].get(), true);
        }

        // --- Re-fetch activeEditor pointer *after* potential detachment and re-adding
        //     Or better, use the 'activeEditor' variable from the block above.
        //     We need the pointer stored in dexedEditors[i] and used for setSize.
        //     Let's reuse the variable 'activeEditor' if it's valid.
        // ---
        if (activeEditor) // Use the pointer obtained earlier
        {
             dexedEditors[i] = activeEditor; // Store the pointer
             // Set size using the potentially valid editor pointer
             dexedComponents[i]->setSize(dexedEditors[i]->getWidth(), dexedEditors[i]->getHeight());
             tabbedComponent->setSize(dexedComponents[i]->getWidth(), dexedComponents[i]->getHeight() + tabbedComponent->getTabBarDepth());
        } else {
            // Handle case where editor was null initially or became problematic
            dexedEditors[i] = nullptr;
            // Optionally set a default size for the tab component if editor is missing
            // tabbedComponent->setSize(600, 400 + tabbedComponent->getTabBarDepth());
        }
    }

    // Make the tabbed component visible
    tabbedComponent->setVisible(true);

    // Set the size of the editor window (Original logic)
    // Note: This uses the size from the *last* iteration's editor/component
    setSize(tabbedComponent->getWidth(), tabbedComponent->getHeight() + 100);

    // Sliders for the MultiDexed parameters
    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    // Missing label addAndMakeVisible and attachToComponent in original code
    detuneSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "detuneSpread", detuneSlider);

    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "panSpread", panSlider);
    addAndMakeVisible(panLabel); // This was present in original .h but not added/attached in original .cpp constructor
    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.attachToComponent(&panSlider, false); // This was present in original .h but not added/attached in original .cpp constructor

}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    // Clean up Dexed components and detach slider attachments
    // Note: Original code had this loop condition using audioProcessor.numberOfInstances,
    //       which is fine. Using the constant from the header is also okay if it matches.
    // Let's stick to the original code's member variable access.
    for (int i = 0; i < audioProcessor.numberOfInstances; i++) {
        // No need to manually delete unique_ptr contents, but nulling pointers is okay
        dexedEditors[i] = nullptr; // Null out the raw pointer
        dexedComponents[i] = nullptr; // Reset the unique_ptr for the container
    }

    tabbedComponent = nullptr; // Reset the unique_ptr for the tabbed component
    detuneSliderAttachment = nullptr; // Reset slider attachments first
    panSliderAttachment = nullptr;
}

//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    // Original paint function was empty
    // It's generally good practice to fill the background:
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    // Original resized() logic:
    panSlider.setBounds(0, 0, 100, 100);
    detuneSlider.setBounds(100, 0, 100, 100);


    // Add tabbed component to hold the Dexed editors
    // Original logic positions it below the sliders' initial area
    tabbedComponent->setBounds(0, 100, getWidth(), getHeight() - 100);

    // NOTE: The original resized() did NOT resize the components *inside* the tabs.
    // This means the Dexed editors would likely not resize correctly if the main window
    // was resized. The fix for the build error C2039 addressed this, but per your request,
    // that fix is NOT included here, reverting to the original potentially problematic resizing.
}
