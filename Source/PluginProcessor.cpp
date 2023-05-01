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

    // Create a AudioPluginInstances from the pluginDescriptions
    // and put them in the dexedPluginInstances array
    for (int i = 0; i < numberOfInstances; i++) {
        dexedPluginInstances[i] = pluginFormatManager.createPluginInstance(
                *pluginDescriptions[0], getSampleRate(), getBlockSize(), msg);
    }

    // Check that the AudioPluginInstances were created, if not print an error
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            std::cout << msg.toStdString() << std::endl;
            return;
        }
    }

    for (int i = 0; i < numberOfInstances; i++) {
        // jassert the existence of the AudioPluginInstance
        jassert(dexedPluginInstances[i] != nullptr);
    }

    // Print the plugin name and vendor for each plugin instance
    for (int i = 0; i < numberOfInstances; i++) {
        std::cout << "Plugin Name: " << dexedPluginInstances[i]->getName().toStdString() << std::endl;
    }
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    // Release the plugins
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] != nullptr) {
            dexedPluginInstances[i]->releaseResources();
        }
    }
}

void PluginAudioProcessor::detune()
{
    // Detune the plugin instances in the range between 0.4 and 0.6
    for (int i = 1; i < numberOfInstances; i++) {
        float range = 0.6 - 0.4;
        float detune = 0.4 + i * range/numberOfInstances;
        dexedPluginInstances[i]->getParameters()[3]->setValueNotifyingHost(detune);
    }

    // Set volume of some plugin instances to 0;
    // 8 instances playing in unison is too much
    dexedPluginInstances[2]->getParameters()[2]->setValueNotifyingHost(0);
    dexedPluginInstances[3]->getParameters()[2]->setValueNotifyingHost(0);
    dexedPluginInstances[4]->getParameters()[2]->setValueNotifyingHost(0);
    dexedPluginInstances[7]->getParameters()[2]->setValueNotifyingHost(0); 

}

void PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int maximumExpectedSamplesPerBlock = samplesPerBlock;

    
    for (int i = 0; i < numberOfInstances; i++) {
        
        if (dexedPluginInstances[i] == nullptr) {
            return;
        }

        dexedPluginInstances[i]->releaseResources();
        dexedPluginInstances[i]->setRateAndBufferSizeDetails(sampleRate, maximumExpectedSamplesPerBlock);

        // sync number of buses

        // TODO: Do we need nuberIfInstances instead of the hardcoded 2?
        for (int dir = 0; dir < 2; ++dir) {
            const bool isInput = (dir == 0);
            int expectedNumBuses = getBusCount(isInput);
            int requiredNumBuses1 = dexedPluginInstances[i]->getBusCount(isInput);

            for (; expectedNumBuses < requiredNumBuses1; expectedNumBuses++)
                dexedPluginInstances[i]->addBus(isInput);

            for (; requiredNumBuses1 < expectedNumBuses; requiredNumBuses1++)
                dexedPluginInstances[i]->removeBus(isInput);

        }

        dexedPluginInstances[i]->setBusesLayout(getBusesLayout());
        dexedPluginInstances[i]->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);

        // Set the program for each plugin instance
        dexedPluginInstances[i]->setCurrentProgram(5);        

    }

    // Get the plugin state from the first plugin instance
    // and apply it to all plugin instances
    juce::MemoryBlock state;
    dexedPluginInstances[0]->getStateInformation(state);
    for (int i = 1; i < numberOfInstances; i++) {
        dexedPluginInstances[i]->setStateInformation(state.getData(), state.getSize());
    }

    // Configure the plugin instances to our liking

    detune();
    
    for (int i = 0; i < dexedPluginInstances[0]->getParameters().size(); i++) {
        // Print the names of the parameters and their values
        // if (!dexedPluginInstances[0]->getParameterName(i).contains("MIDI CC")) {
        //     std::cout << "Parameter " << i << ": " << dexedPluginInstances[0]->getParameterName(i).toStdString() << " = " << dexedPluginInstances[0]->getParameter(i) << std::endl;
        // }
        // Add listener to each parameter
        juce::AudioProcessorParameter* parameter = dexedPluginInstances[0]->getParameters()[i];
        parameter->addListener(this);
    
    }   
     
}

void PluginAudioProcessor::releaseResources()
{
    // Release the plugins
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] != nullptr) {
            dexedPluginInstances[i]->releaseResources();
        }
    }
}

void PluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                        juce::MidiBuffer &midiMessages)
{
    int numberOfUnmutedInstances = 0;
    for (int i = 1; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i]->getParameters()[2]->getValue()>0) {
            numberOfUnmutedInstances++;
        }
    }
    for (int i = 1; i < numberOfInstances; i++) {
        // Make empty initialized buffer for each plugin instance in dexedPluginBuffers
        dexedPluginBuffers[i] = juce::AudioBuffer<float>(buffer.getNumChannels(), buffer.getNumSamples());
        // Process the audio through each plugin instance
        if (dexedPluginInstances[i]) {
            dexedPluginInstances[i]->processBlock(dexedPluginBuffers[i], midiMessages);
        }
    }

    // Combine the sound of all the plugin instances
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float sum = 0;
            int i;
            for (i = 1; i < numberOfInstances; i++) {
                sum += dexedPluginBuffers[i].getSample(channel, sample);
                // Pan instances; instance 1 is full left, 2 a bit less left, ... , last instance is full right
                float pan = (float)i / (numberOfInstances - 3);
                if (channel == 0) {
                    sum += dexedPluginBuffers[i].getSample(channel, sample) * (1 - pan);
                }
                else {
                    sum += dexedPluginBuffers[i].getSample(channel, sample) * pan;
                }
            }

            buffer.setSample(channel, sample, sum / numberOfUnmutedInstances);

        }
    }


    // 

}

juce::AudioProcessorEditor *PluginAudioProcessor::createEditor()
{
    return new PluginAudioProcessorEditor(*this);
}

void PluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // Return state of instance 0
    if (dexedPluginInstances[0] != nullptr) {
        return dexedPluginInstances[0]->getStateInformation(destData);
    }
    // TODO: Should probably save the state of all instances individually;
    // how do other plugin hosts save the state of multiple plugin instances?
}

void PluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes) { 
    // Set state of all instances, but prevent infinite loop
    if (dexedPluginInstances[0] != nullptr) {
        shouldSynchronize = false;
        for (int i = 0; i < numberOfInstances; i++) {
            dexedPluginInstances[i]->setStateInformation(data, sizeInBytes);
        }
        detune();
        shouldSynchronize = true;
    }
    // TODO: Should probably load the state of all instances from the saved state individually;
    // how do other plugin hosts save the state of multiple plugin instances?
}

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
    return dexedPluginInstances[0]->getTailLengthSeconds();
}

int PluginAudioProcessor::getNumPrograms()
{
    return dexedPluginInstances[0]->getNumPrograms();
}

int PluginAudioProcessor::getCurrentProgram()
{
    return dexedPluginInstances[0]->getCurrentProgram();
}

// setCurrentProgram() is called when the user changes the program in the host
void PluginAudioProcessor::setCurrentProgram(int index)
{
    // Update the program in instance 0, the other instances will follow
    dexedPluginInstances[0]->setCurrentProgram(index);
}

const juce::String PluginAudioProcessor::getProgramName(int index)
{
    return dexedPluginInstances[0]->getProgramName(index);
}

void PluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {
    dexedPluginInstances[0]->changeProgramName(index, newName);
}

bool PluginAudioProcessor::hasEditor() const
{
    // Only permit editor to open if plugins instantiated properly
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return false;
        }
    }
    return true;
}

bool PluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    return (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

// Because we inherit from AudioProcessorValueTreeState::Listener, we need to implement this method
void PluginAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    // Get the parameter that changed
    juce::AudioProcessorParameter* parameter = dexedPluginInstances[0]->getParameters()[parameterIndex];
    // Get the name of the parameter
    juce::String parameterName = parameter->getName(100);

    std::cout << "Parameter " << parameterIndex << ": " << parameterName.toStdString() << " = " << newValue << std::endl;

    // Update the value of the parameter in all other plugin instances
    for (int i = 1; i < numberOfInstances; i++) {
        if (shouldSynchronize) {
            juce::AudioProcessorParameter* parameter = dexedPluginInstances[i]->getParameters()[parameterIndex];
            parameter->setValueNotifyingHost(newValue);
        }
        // FIXME: Why does the above work for some parameters but not others (e.g. "OP1 F COARSE")?
    }

    // When a cartridge is loaded, update the parameters of all instances
    // TODO: Find a better trigger for this, e.g. when the user clicks "Load Cartridge"
    if (parameterIndex == 2236) {
        // Synchronize the plugin state from instance 0 to all other instances
        // Get the state of instance 0
        juce::MemoryBlock state;
        dexedPluginInstances[0]->getStateInformation(state);
        for (int i = 1; i < numberOfInstances; i++) {
            dexedPluginInstances[i]->setStateInformation(state.getData(), state.getSize());
        }
        detune();
        // Update the names of all programs exposed by the plugin to the host
        updateHostDisplay(); // TODO: Why does this not work? How can we update the menu containing the progams in the host?
        // dexedPluginInstances[0]->updateHostDisplay(); // Does not work either

        // Change the program to the one selected in instance 0
        setCurrentProgram(dexedPluginInstances[0]->getCurrentProgram());
    }
}

// Because we inherit from AudioProcessorValueTreeState::Listener, we need to implement this method
void PluginAudioProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    // Not used
}