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
    // Using the 'audioProcessor' reference member is direct and efficient.
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
            dexedEditors[i] = activeEditor; // Store the pointer for later reference if needed

            // ---> FIX ATTEMPT: Explicitly remove from any potential old parent <---
            // If the editor still thinks it has a parent from the *previous* window instance,
            // this might help detach it cleanly before adding it to the new hierarchy.
            if (auto* oldParent = activeEditor->getParentComponent()) {
                // Optionally log this: DBG("Removing editor from old parent: " + oldParent->getName());
                oldParent->removeChildComponent(activeEditor);
            }
            // ---> END FIX ATTEMPT <---

            // Now add the Dexed editor to our new container component for this tab
            dexedComponents[i]->addAndMakeVisible(activeEditor);

            // Size the container component to match the Dexed editor's size.
            // Use a default if the editor reports zero size initially.
            int editorWidth = activeEditor->getWidth() > 0 ? activeEditor->getWidth() : 600;
            int editorHeight = activeEditor->getHeight() > 0 ? activeEditor->getHeight() : 400;
            dexedComponents[i]->setSize(editorWidth, editorHeight);

            // Ensure the Dexed editor itself fills its container.
            // This is often handled by addAndMakeVisible depending on component settings,
            // but explicit setBounds is safest here.
            activeEditor->setBounds(0, 0, editorWidth, editorHeight);

            // Add the container component (which holds the Dexed editor) as a tab page
            juce::String tabName = (i == 0) ? "Master" : juce::String("Dexed ") + juce::String(i);
            tabbedComponent->addTab(tabName, backgroundColor, dexedComponents[i].get(), true);

        } else {
            // Error handling: Could not get the editor for this Dexed instance
            DBG("Error: Could not get active editor for Dexed instance " + juce::String(i));
            dexedEditors[i] = nullptr; // Ensure editor pointer is null
            // Add a tab indicating the error, perhaps with no component or a placeholder
            juce::String tabName = (i == 0) ? "Master (Error)" : juce::String("Dexed ") + juce::String(i) + " (Error)";
            // You could create a simple Label component here to show the error, or add null:
            tabbedComponent->addTab(tabName, juce::Colours::red, nullptr, true); // Adds tab with no content
        }
    } // --- END OF THE INSTANCE LOOP ---

    // --- Determine Overall Size (Moved Outside Loop) ---
    int finalTabContentWidth = 600;  // Default width
    int finalTabContentHeight = 400; // Default height
    int tabBarDepth = tabbedComponent->getTabBarDepth();

    // Find the size of the first validly loaded Dexed editor to set the initial size.
    // Iterate through the stored editor pointers.
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; ++i) {
        if (dexedEditors[i] != nullptr && dexedEditors[i]->getWidth() > 0 && dexedEditors[i]->getHeight() > 0) {
            finalTabContentWidth = dexedEditors[i]->getWidth();
            finalTabContentHeight = dexedEditors[i]->getHeight();
            break; // Use the dimensions of the first one found
        }
    }

    // Set the size of the tabbed component itself
    // Width = content width, Height = content height + tab bar depth
    tabbedComponent->setSize(finalTabContentWidth, finalTabContentHeight + tabBarDepth);

    // Set the total size of this main plugin editor window
    // Typically TabbedComponent width, and TabbedComponent height + space for other controls
    const int controlsAreaHeight = 100; // Height reserved for sliders below tabs
    setSize(tabbedComponent->getWidth(), tabbedComponent->getHeight() + controlsAreaHeight);
    // --- End Sizing Logic ---


    // Sliders for the MultiDexed parameters (placed below the tabs)
    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    detuneSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "detuneSpread", detuneSlider);
    addAndMakeVisible(detuneLabel); // Add the label for detune
    detuneLabel.setText("Detune", juce::dontSendNotification);
    detuneLabel.attachToComponent(&detuneSlider, false); // Attach below slider
    detuneLabel.setJustificationType(juce::Justification::centred); // Center the label text


    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "panSpread", panSlider);
    addAndMakeVisible(panLabel); // Add the label for pan
    panLabel.setText("Pan Spread", juce::dontSendNotification); // Changed text slightly for clarity
    panLabel.attachToComponent(&panSlider, false); // Attach below slider
    panLabel.setJustificationType(juce::Justification::centred); // Center the label text

    // Note: The actual positioning of sliders/labels happens in resized()
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    // Clean up: Detach slider attachments FIRST to avoid dangling pointers
    detuneSliderAttachment = nullptr;
    panSliderAttachment = nullptr;

    // Explicitly null out pointers to the Dexed editors.
    // We don't own these editor objects directly; the AudioPluginInstance does.
    // Setting our pointers to null prevents accidental use after destruction.
    for (int i = 0; i < audioProcessor.numberOfInstances; i++) {
        dexedEditors[i] = nullptr;
    }

    // The unique_ptr members (tabbedComponent, dexedComponents)
    // will automatically delete the objects they manage here.
    // When dexedComponents[i] is destroyed, it will remove and delete
    // the Dexed editor *if* it was added via addAndMakeVisible(editor, true)
    // or if it owns the component. In our case, addAndMakeVisible(editor)
    // doesn't transfer ownership, so Dexed's editor lifecycle is still
    // managed by its AudioPluginInstance. Our unique_ptr cleans up the
    // container Component, not the Dexed editor itself.
    tabbedComponent = nullptr; // unique_ptr reset
    // dexedComponents unique_ptrs reset automatically as member variables
}

//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    // Fill the background, respecting LookAndFeel
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Optionally draw borders or other background elements here if needed
    // This paint() function only draws the area *not* covered by child components
    // (like the tabbedComponent and sliders). In this layout, it will mainly paint
    // the area around the sliders if the window is larger than needed.
}

void PluginAudioProcessorEditor::resized()
{
    // This is where you define the layout of components within this editor window.

    // Define the bounds for the area containing the sliders/labels
    // Let's place them side-by-side at the top, above the tabs
    auto bounds = getLocalBounds(); // Get the total area of this editor window

    // Area for controls (sliders, labels) at the top
    auto controlsArea = bounds.removeFromTop(100); // Use the reserved height

    // Position sliders and labels within the controlsArea
    // Example: Place Detune on the left, Pan on the right
    int sliderWidth = 80;
    int sliderHeight = 80; // Rotary sliders often square-ish
    int labelHeight = 20;

    juce::Rectangle<int> detuneSliderArea = controlsArea.removeFromLeft(sliderWidth + 20); // Add some padding
    detuneSlider.setBounds(detuneSliderArea.getX() + 10, detuneSliderArea.getY(), sliderWidth, sliderHeight);
    // Label positioning handled by attachToComponent

    juce::Rectangle<int> panSliderArea = controlsArea.removeFromLeft(sliderWidth + 20); // Area for the next slider
    panSlider.setBounds(panSliderArea.getX() + 10, panSliderArea.getY(), sliderWidth, sliderHeight);
    // Label positioning handled by attachToComponent


    // The remaining 'bounds' area is now designated for the tabbed component
    // Position the tabbed component to fill the rest of the editor window below controls
    tabbedComponent->setBounds(bounds);

    // Important: Need to resize the *contents* of the tabs too.
    // The TabbedComponent itself resizes, but the components *inside* each tab
    // (our dexedComponents containers) need their bounds set relative to the tab page.
    // We iterate through the tabs and set the bounds of our container component.
    for (int i = 0; i < tabbedComponent->getNumTabs(); ++i) {
        if (auto* tabContentComponent = tabbedComponent->getTabContentComponent(i)) {
             // Make the container fill the content area of the tab page.
             // The TabbedComponent provides the correct bounds for its content area.
             tabContentComponent->setBounds(tabbedComponent->getTabContentAreaBounds());

             // Now ensure the Dexed editor *inside* the container is also resized
             // (assuming it's the only child, index 0).
             if (tabContentComponent->getNumChildComponents() > 0) {
                 if(auto* dexedEditorComp = tabContentComponent->getChildComponent(0)) {
                     // Make the Dexed editor fill its container (which fills the tab)
                     dexedEditorComp->setBounds(tabContentComponent->getLocalBounds());
                 }
             }
        }
    }
}
