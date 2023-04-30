/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 */
class PluginAudioProcessor : public juce::AudioProcessor,
                             juce::AudioProcessorParameter::Listener
                             // https://www.youtube.com/watch?v=Bw_OkHNpj1M&t=1990s
#if JucePlugin_Enable_ARA
    ,
                             public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    PluginAudioProcessor();
    ~PluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    const int numberOfInstances = 9;

    bool shouldSetStateInformation = true;

    // Make an array that can hold numberOfInstances juce::AudioProcessor instances
    std::array<std::unique_ptr<juce::AudioProcessor>, 9> dexedPluginInstances;

    // Buffers for the plugin instances
    std::array<juce::AudioBuffer<float>, 9> dexedPluginBuffers;

    // Because we inherit from juce::AudioProcessorParameter::Listener, we need to implement these methods
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginAudioProcessor)
};
