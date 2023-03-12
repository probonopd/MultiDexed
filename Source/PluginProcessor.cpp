#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

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
    dexedPluginInstance1 = pluginFormatManager.createPluginInstance(
            *pluginDescriptions[0], getSampleRate(), getBlockSize(), msg);
    dexedPluginInstance2 = pluginFormatManager.createPluginInstance(
            *pluginDescriptions[0], getSampleRate(), getBlockSize(), msg);

    // Check that the AudioPluginInstances were created
    jassert(dexedPluginInstance1 != nullptr);
    jassert(dexedPluginInstance2 != nullptr);

    std::cout << "Loaded Plugin: " << dexedPluginInstance1->getName().toStdString() << std::endl;
    std::cout << "Loaded Plugin: " << dexedPluginInstance2->getName().toStdString() << std::endl;
}

PluginAudioProcessor::~PluginAudioProcessor() { }

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
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Allocate memory for the plugin buffer
    juce::AudioBuffer<float> dexedPluginBuffer1(numChannels, numSamples);

    // Initialize the plugin buffer
    dexedPluginBuffer1.clear();

    // Set the size of the plugin buffer
    dexedPluginBuffer1.setSize(numChannels, numSamples);

    // Process the buffer with the first plugin
    dexedPluginInstance1->processBlock(dexedPluginBuffer1, midiMessages);

    // Print the values in the plugin buffer

    for (int channel = 0; channel < dexedPluginBuffer1.getNumChannels(); ++channel) {
        for (int sample = 0; sample < dexedPluginBuffer1.getNumSamples(); ++sample) {
            std::printf("%02x ",
                        (int)(dexedPluginBuffer1.getSample(channel, sample) * 127.0f + 128.0f));
        }
    }

    // The values we are seeing (80 80 80 80 80 80 80 80 80 80 ...) suggest that the samples in
    // pluginBuffer1 are all around the 0.0 value. This could mean that there is an issue with the
    // configuration of dexedPluginInstance1, or that the MIDI messages being sent to it are not
    // producing any meaningful audio output.

    // Process each sample in the buffer
    for (int channel = 0; channel < numChannels; ++channel) {
        for (int sample = 0; sample < numSamples; ++sample) {
            // Get the sample from the first plugin
            const float value = dexedPluginBuffer1.getSample(channel, sample);
            // Set the sample in the buffer
            buffer.setSample(channel, sample, value);
        }
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
    return true;
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
