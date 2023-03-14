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
{


}

PluginAudioProcessor::~PluginAudioProcessor()
{
    // Release the plugins
    dexedPluginInstance1->releaseResources();
    dexedPluginInstance2->releaseResources();
}

void PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int maximumExpectedSamplesPerBlock = samplesPerBlock;

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
    dexedPluginInstance1->releaseResources();
    dexedPluginInstance2->releaseResources();
}

void PluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                        juce::MidiBuffer &midiMessages)
{
    // Process the audio through the first plugin
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
