#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>


// Build on FreeBSD with:
// sed -i '' -e 's|stat64|stat|g'  make_helpers/juce_SimpleBinaryBuilder.cpp
// gmake CONFIG=Release
// or
// gmake CONFIG=Debug

PluginAudioProcessor::PluginAudioProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    juce::OwnedArray<juce::PluginDescription> pluginDescriptions;
    juce::KnownPluginList pluginList;
    juce::AudioPluginFormatManager pluginFormatManager;

    juce::VST3PluginFormat *vst3 = new juce::VST3PluginFormat();
    pluginFormatManager.addFormat(vst3);

    juce::String pluginPath;

    // Check which operating system we are running on
    // and set the plugin path accordingly

    // Linux
    if (juce::SystemStats::getOperatingSystemType() & juce::SystemStats::OperatingSystemType::Linux) {
        pluginPath = "/usr/lib/vst3/Dexed.vst3";
    }

    // Windows
    if (juce::SystemStats::getOperatingSystemType() & juce::SystemStats::OperatingSystemType::Windows) {
        pluginPath = "C:\\Program Files\\Common Files\\VST3\\Dexed.vst3";
    }

    // MacOS
    if (juce::SystemStats::getOperatingSystemType() & juce::SystemStats::OperatingSystemType::MacOSX) {
        pluginPath = "/Library/Audio/Plug-Ins/VST3/Dexed.vst3";
    }

    // FreeBSD
    if (juce::SystemStats::getOperatingSystemType() == juce::SystemStats::OperatingSystemType::UnknownOS) {
        pluginPath = "/usr/local/lib/vst3/Dexed.vst3";
    }

    // Print the plugin path or error if not found
    if (pluginPath.isEmpty()) {
        std::cout << "Error: Plugin not found" << std::endl;

    } else {
        std::cout << "Plugin Path: " << pluginPath.toStdString() << std::endl;
    }
    
    pluginList.scanAndAddFile(pluginPath, true, pluginDescriptions,
                              *pluginFormatManager.getFormat(0));

    // If no plugin was found, print an error
    if (pluginDescriptions.size() == 0) {
        std::cout << "Error: Dexed plugin not found" << std::endl;
        return;
    }

    juce::String msg("Error Loading Plugin: ");

    // Create a AudioPluginInstance for each plugin
    dexedPluginInstance1 = pluginFormatManager.createPluginInstance(
            *pluginDescriptions[0], getSampleRate(), getBlockSize(), msg);
    dexedPluginInstance2 = pluginFormatManager.createPluginInstance(
            *pluginDescriptions[0], getSampleRate(), getBlockSize(), msg);

    // Check that the AudioPluginInstances were created, if not print an error
    if (dexedPluginInstance1 == nullptr || dexedPluginInstance2 == nullptr) {
        std::cout << msg.toStdString() << std::endl;
        return;
    }

    jassert(dexedPluginInstance1);
    jassert(dexedPluginInstance2);

    std::cout << "Loaded Plugin: " << dexedPluginInstance1->getName().toStdString() << std::endl;
    std::cout << "Loaded Plugin: " << dexedPluginInstance2->getName().toStdString() << std::endl;
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    // Release the plugins
    if (dexedPluginInstance1)
        dexedPluginInstance1->releaseResources();
    if (dexedPluginInstance2)
        dexedPluginInstance2->releaseResources();
}

void PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int maximumExpectedSamplesPerBlock = samplesPerBlock;

    if (!dexedPluginInstance1 || !dexedPluginInstance2)
        return;

    dexedPluginInstance1->releaseResources();
    dexedPluginInstance1->setRateAndBufferSizeDetails(sampleRate, maximumExpectedSamplesPerBlock);
    dexedPluginInstance2->releaseResources();
    dexedPluginInstance2->setRateAndBufferSizeDetails(sampleRate, maximumExpectedSamplesPerBlock);

    // sync number of buses
    for (int dir = 0; dir < 2; ++dir) {
        const bool isInput = (dir == 0);
        int expectedNumBuses = getBusCount(isInput);
        int requiredNumBuses1 = dexedPluginInstance1->getBusCount(isInput);
        int requiredNumBuses2 = dexedPluginInstance2->getBusCount(isInput);

        for (; expectedNumBuses < requiredNumBuses1; expectedNumBuses++)
            dexedPluginInstance1->addBus(isInput);
        for (; expectedNumBuses < requiredNumBuses2; expectedNumBuses++)
            dexedPluginInstance2->addBus(isInput);

        for (; requiredNumBuses1 < expectedNumBuses; requiredNumBuses1++)
            dexedPluginInstance1->removeBus(isInput);
        for (; requiredNumBuses2 < expectedNumBuses; requiredNumBuses2++)
            dexedPluginInstance2->removeBus(isInput);
    }

    dexedPluginInstance1->setBusesLayout(getBusesLayout());
    dexedPluginInstance1->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);

    dexedPluginInstance2->setBusesLayout(getBusesLayout());
    dexedPluginInstance2->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void PluginAudioProcessor::releaseResources()
{
    // Release the plugins
    if (dexedPluginInstance1)
        dexedPluginInstance1->releaseResources();
    if (dexedPluginInstance2)
        dexedPluginInstance2->releaseResources();
}

void PluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                        juce::MidiBuffer &midiMessages)
{
    // Process the audio through the first plugin
    if (dexedPluginInstance1)
        dexedPluginInstance1->processBlock(buffer, midiMessages);
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
    // Only permit editor to open if plugins instantiated properly
    return (dexedPluginInstance1 && dexedPluginInstance2);
}

bool PluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    return (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}
