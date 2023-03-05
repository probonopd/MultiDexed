#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

// Build on FreeBSD with:
// cd Builds/LinuxMakefile
// sed -i '' -e 's|stat64|stat|g'  make_helpers/juce_SimpleBinaryBuilder.cpp
// gmake CONFIG=Release
// or
// gmake CONFIG=Debug

PluginAudioProcessor::PluginAudioProcessor()
{

    juce::OwnedArray<juce::PluginDescription> pluginDescriptions;
    juce::KnownPluginList pluginList;
    juce::AudioPluginFormatManager pluginFormatManager;

    juce::VST3PluginFormat *vst3 = new juce::VST3PluginFormat();
    pluginFormatManager.addFormat(vst3);

    juce::String pluginPath("C:\\Program Files\\Common Files\\VST3\\Dexed.vst3");

    pluginList.scanAndAddFile(pluginPath, true, pluginDescriptions,
                              *pluginFormatManager.getFormat(0));

    jassert(pluginDescriptions.size() > 0);
    juce::String msg("Error Loading Plugin: ");

    // Create a AudioPluginInstance for each plugin
    auto dexedPluginInstance1 = pluginFormatManager.createPluginInstance(
            *pluginDescriptions[0], getSampleRate(), getBlockSize(), msg);
    auto dexedPluginInstance2 = pluginFormatManager.createPluginInstance(
            *pluginDescriptions[0], getSampleRate(), getBlockSize(), msg);

    // Check that the AudioPluginInstances were created
    jassert(dexedPluginInstance1 != nullptr);
    jassert(dexedPluginInstance2 != nullptr);
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    // Release the plugins
    dexedPluginInstance1->releaseResources();
    dexedPluginInstance2->releaseResources();
}

void PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Update the sample rate and block size
    setRateAndBufferSizeDetails(sampleRate, samplesPerBlock);

    // Update the plugin buffer sizes
    dexedPluginInstance1->prepareToPlay(sampleRate, samplesPerBlock);
    dexedPluginInstance2->prepareToPlay(sampleRate, samplesPerBlock);
}

void PluginAudioProcessor::releaseResources()
{
    // Release the plugins
    dexedPluginInstance1->releaseResources();
    dexedPluginInstance2->releaseResources();
}

void PluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                        juce::MidiBuffer &midiMessages)
{
    // Process the audio through the first plugin
    juce::AudioBuffer<float> plugin1Buffer(buffer);
    plugin1Buffer.clear();
    dexedPluginInstance1->processBlock(plugin1Buffer, midiMessages);

    // Process the audio through the second plugin
    juce::AudioBuffer<float> plugin2Buffer(buffer);
    plugin2Buffer.clear();
    dexedPluginInstance2->processBlock(plugin2Buffer, midiMessages);

    // Combine the output of the two plugins
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        buffer.addFrom(channel, 0, plugin1Buffer.getReadPointer(channel), buffer.getNumSamples());
        buffer.addFrom(channel, 0, plugin2Buffer.getReadPointer(channel), buffer.getNumSamples());
    }
}

juce::AudioProcessorEditor *PluginAudioProcessor::createEditor()
{
    return new PluginAudioProcessorEditor(*this);
}

void PluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) { }

void PluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes) { }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PluginAudioProcessor();
}

const juce::String PluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0
              // programs, so this should be at least 1, even if you're not really
              // implementing programs.
}

int PluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginAudioProcessor::setCurrentProgram(int index) { }

const juce::String PluginAudioProcessor::getProgramName(int index)
{
    return {};
}

void PluginAudioProcessor::changeProgramName(int index, const juce::String &newName) { }

bool PluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#  if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#  else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#    if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#    endif

    return true;
#  endif
}
#endif
