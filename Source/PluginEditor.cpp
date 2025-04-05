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
    auto* pluginAudioProcessor = &audioProcessor; // Use direct reference

    // Create a tabbed component
    tabbedComponent = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(*tabbedComponent);

    auto backgroundColor = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);

    // Create tabs and container components, but don't load editors yet
    for (int i = 0; i < pluginAudioProcessor->numberOfInstances; i++)
    {
         // Safety check against array size declared in .h
         if (i >= 8) {
              jassertfalse; // numberOfInstances exceeds array size in .h
              break;
         }

        dexedComponents[i] = std::make_unique<juce::Component>();
        // Set name for debugging visibility if needed: dexedComponents[i]->setName("Container " + juce::String(i));

        // Don't get/add the actual Dexed editor here anymore

        juce::String tabName = (i == 0) ? "Master" : juce::String("Dexed ") + juce::String(i);
        tabbedComponent->addTab(tabName, backgroundColor, dexedComponents[i].get(), false); // Add container, don't delete on removal initially
    }

    // --- Add Listener ---
    tabbedComponent->addListener(this);

    // Initial setup sizing (can be refined) - Use a default or first instance's potential size
    // This might need adjustment based on when Dexed reports its size correctly.
    int initialWidth = 600; // Default size
    int initialHeight = 400;
     // We could try getting size from instance 0 if it's reliable here:
     // if(pluginAudioProcessor->dexedPluginInstances[0]) {
     //    pluginAudioProcessor->dexedPluginInstances[0]->createEditorIfNeeded();
     //    if(auto* ed = pluginAudioProcessor->dexedPluginInstances[0]->getActiveEditor()) {
     //        initialWidth = ed->getWidth(); initialHeight = ed->getHeight();
     //    }
     // }
    tabbedComponent->setSize(initialWidth, initialHeight + tabbedComponent->getTabBarDepth());

    // Set initial size of the editor window
    const int controlsAreaHeight = 100;
    setSize(tabbedComponent->getWidth(), tabbedComponent->getHeight() + controlsAreaHeight);


    // Sliders setup (ensure labels are added and attached)
    addAndMakeVisible(detuneSlider);
    detuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    detuneSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "detuneSpread", detuneSlider);
    addAndMakeVisible(detuneLabel); // Add label
    detuneLabel.setText("Detune", juce::dontSendNotification);
    detuneLabel.attachToComponent(&detuneSlider, false); // Attach below
    detuneLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(panSlider);
    panSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, true, 50, 20);
    panSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(pluginAudioProcessor->apvts, "panSpread", panSlider);
    addAndMakeVisible(panLabel); // Add label
    panLabel.setText("Pan Spread", juce::dontSendNotification);
    panLabel.attachToComponent(&panSlider, false); // Attach below
    panLabel.setJustificationType(juce::Justification::centred);

    // --- Trigger loading the content for the initially selected tab ---
    // Ensure resized() has been called at least once before triggering this if it relies on layout.
    // Might be safer to trigger via an async update or timer if needed, but direct call often works.
    if (tabbedComponent->getNumTabs() > 0) {
        // Set current tab AFTER attaching listener and potentially setting initial size
        tabbedComponent->setCurrentTabIndex(0, false); // Select first tab, don't send notification yet
        currentTabChanged(0, tabbedComponent->getTabNames()[0]); // Manually call to load content
    }
    tabbedComponent->setVisible(true); // Already done by addAndMakeVisible, but safe.
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    // --- Remove Listener ---
    tabbedComponent->removeListener(this);

    detuneSliderAttachment = nullptr;
    panSliderAttachment = nullptr;

    // unique_ptrs for tabbedComponent and dexedComponents handle their own cleanup.
    // When dexedComponents[i] is deleted, its children (the Dexed editor added dynamically)
    // should be handled correctly by JUCE's component hierarchy.
}

//==============================================================================
void PluginAudioProcessorEditor::currentTabChanged (int newCurrentTabIndex, const juce::String& /*newCurrentTabName*/)
{
    // Check if the index is valid for our processor instances and component arrays
    if (newCurrentTabIndex < 0 || newCurrentTabIndex >= audioProcessor.numberOfInstances || newCurrentTabIndex >= 8) {
         jassertfalse; // Index out of bounds
         return;
    }

    auto* container = dexedComponents[newCurrentTabIndex].get();
    if (!container) {
        DBG("Error: Container component for tab " + juce::String(newCurrentTabIndex) + " is null.");
        return; // Container doesn't exist
    }

    auto* processorInstance = audioProcessor.dexedPluginInstances[newCurrentTabIndex].get();
    if (!processorInstance) {
         DBG("Error: Processor instance for tab " + juce::String(newCurrentTabIndex) + " is null.");
         container->deleteAllChildren(); // Clear container if processor invalid
         return; // Processor instance doesn't exist
    }

    // Ensure editor exists and get it
    processorInstance->createEditorIfNeeded();
    auto* editor = processorInstance->getActiveEditor();

    // Clear any previous component from the container
    container->deleteAllChildren(); // Remove whatever was there before

    if (editor)
    {
        // Add the editor to the container
        container->addAndMakeVisible(editor);
        // Resize the editor to fill the container (whose size is set in the main 'resized()' method)
        editor->setBounds(container->getLocalBounds());

        // Store the pointer if needed (though fetching it here might be sufficient)
        dexedEditors[newCurrentTabIndex] = editor;
    } else {
        DBG("Warning: Could not get active editor for instance " + juce::String(newCurrentTabIndex));
        dexedEditors[newCurrentTabIndex] = nullptr;
        // Optionally add a placeholder Label to the container indicating the error
        // auto* errorLabel = new juce::Label("err", "Editor Failed to Load");
        // container->addAndMakeVisible(errorLabel);
        // errorLabel->setBounds(container->getLocalBounds());
    }
}


//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto controlsArea = bounds.removeFromTop(100); // Area for sliders

    // Layout sliders
    int sliderWidth = 80;
    int sliderHeight = 80;
    juce::Rectangle<int> detuneSliderArea = controlsArea.removeFromLeft(sliderWidth + 20);
    detuneSlider.setBounds(detuneSliderArea.getX() + 10, detuneSliderArea.getY(), sliderWidth, sliderHeight);
    juce::Rectangle<int> panSliderArea = controlsArea.removeFromLeft(sliderWidth + 20);
    panSlider.setBounds(panSliderArea.getX() + 10, panSliderArea.getY(), sliderWidth, sliderHeight);
    // Labels are positioned by attachToComponent

    // Layout tabbed component
    tabbedComponent->setBounds(bounds);

    // --- IMPORTANT: Resize the *CONTAINER* components within the tabs ---
    // This uses the corrected logic to calculate the content area.
    for (int i = 0; i < tabbedComponent->getNumTabs(); ++i) {
        // Get the container component associated with the tab index 'i'
        if (auto* tabContentComponent = tabbedComponent->getTabContentComponent(i))
        {
             // Calculate the bounds available for the tab's content (area below the tab bar)
             auto contentBounds = tabbedComponent->getLocalBounds();
             contentBounds.removeFromTop(tabbedComponent->getTabBarDepth());

             // Set the bounds of our container component (e.g., dexedComponents[i])
             tabContentComponent->setBounds(contentBounds);

             // The editor *inside* the container also needs resizing.
             // This will happen when the tab is selected via currentTabChanged,
             // but we can also do it here for the *currently visible* editor.
             if (i == tabbedComponent->getCurrentTabIndex() && tabContentComponent->getNumChildComponents() > 0) {
                 if(auto* currentEditor = tabContentComponent->getChildComponent(0)) {
                     currentEditor->setBounds(tabContentComponent->getLocalBounds()); // Make editor fill container
                 }
             }
             // Alternatively, could force a call to currentTabChanged for the current index here,
             // but resizing the direct child is usually sufficient if the container is sized correctly.
        }
    }
    // --- End Content Resizing ---
}
