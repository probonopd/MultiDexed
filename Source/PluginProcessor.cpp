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
    : apvts(*this, nullptr, "Parameters", createParameterLayout()),
      // juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
    juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
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
        pluginPath = "C:\\Program Files\\Common Files\\VST3\\Dexed.vst3\\Contents\\x86_64-win\\Dexed.vst3";
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

    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return;
        }
    }

    // Detune the plugin instances in the range determined by the detuneSpread parameter
    float range = apvts.getRawParameterValue("detuneSpread")->load();
    std::cout << "Using Detune Spread: " << range << std::endl;
    for (int i = 1; i < numberOfInstances; i++) {
        double detune = 0.5 - range/2.0 + i * range/numberOfInstances;
        std::cout << "Setting instance " << i << " to detune " << detune << std::endl;
        dexedPluginInstances[i]->getParameters()[3]->setValueNotifyingHost(detune);
    }

}

void PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return;
        }
    }

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
        dexedPluginInstances[i]->setStateInformation(state.getData(), static_cast<size_t>(state.getSize()));
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

    // Add apvts listener for detuneSpread in order to call detune() when it changes
    apvts.addParameterListener("detuneSpread", this);

    // Add apvts listener for panSpread in order to print a message when it changes
    apvts.addParameterListener("panSpread", this);

    // Add apvts listener for all parameters to update the other instances when they change
    // for (int i = 0; i < getParameters().size(); i++) {
    //     juce::AudioProcessorParameter* parameter = getParameters()[i];
    //     apvts.addParameterListener(parameter->getName(0), this);
    // }

    // Add aptvs listener for all parameters in instance 0 in order to update the other instances when they change
    // for (int i = 0; i < dexedPluginInstances[0]->getParameters().size(); i++) {
    //     juce::AudioProcessorParameter* parameter = dexedPluginInstances[0]->getParameters()[i];
    //     apvts.addParameterListener(parameter->getName(0), this);
    // }
  
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
    for (int i = 0; i < numberOfInstances; i++) {
        // NOTE: Even though we don't use the sound of plugin instance 0, we still need to process it for the GUI to work
        // Make empty initialized buffer for each plugin instance in dexedPluginBuffers
        dexedPluginBuffers[i] = juce::AudioBuffer<float>(buffer.getNumChannels(), buffer.getNumSamples());
        // Process the audio through each plugin instance
        if (dexedPluginInstances[i]) {
            dexedPluginInstances[i]->processBlock(dexedPluginBuffers[i], midiMessages);
        }
    }

    // TODO: If we don't want artifacts when panSpread is automated,
    // we need to make sure that the panSpread value gets smoothed between its old and new value?
    float panAmountFactor = apvts.getRawParameterValue("panSpread")->load();
    // std::cout << "Using Pan Spread: " << panAmountFactor << std::endl;
    // Combine the sound of all the plugin instances
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            double sum = 0;
            int i;
            for (i = 1; i < numberOfInstances; i++) {
                // if numberOfInstances is 9, pan for instance 1 is 0.0, for instance 2 is 0.14, for instance 3 is 0.28, for instance 4 is 0.42, for instance 5 is 0.57, for instance 6 is 0.71, for instance 7 is 0.85, for instance 8 is 1.0
                // if numberOfInstances is 8, pan for instance 1 is 0.0, for instance 2 is 0.17, for instance 3 is 0.33, for instance 4 is 0.5, for instance 5 is 0.67, for instance 6 is 0.83, for instance 7 is 1.0
                // if numberOfInstances is 7, pan for instance 1 is 0.0, for instance 2 is 0.2, for instance 3 is 0.4, for instance 4 is 0.6, for instance 5 is 0.8, for instance 6 is 1.0
                // if numberOfInstances is 6, pan for instance 1 is 0.0, for instance 2 is 0.25, for instance 3 is 0.5, for instance 4 is 0.75, for instance 5 is 1.0
                // if numberOfInstances is 5, pan for instance 1 is 0.0, for instance 2 is 0.33, for instance 3 is 0.66, for instance 4 is 1.0
                // if numberOfInstances is 4, pan for instance 1 is 0.0, for instance 2 is 0.5, for instance 3 is 1.0
                // if numberOfInstances is 3, pan for instance 1 is 0.0, for instance 2 is 1.0
                // if numberOfInstances is 2, pan for instance 1 is 0.0
                // if numberOfInstances is 1, pan for instance 1 is 0.0
                // Considering the above, the pan for instance i is (i-1)/(numberOfInstances-1)
                double pan = (i-1.0)/(numberOfInstances-1.0);
                
                // if (channel == 0) {
                //     // Left channel
                //     sum += dexedPluginBuffers[i].getSample(channel, sample) * (1.0 - pan);
                // }
                // else {
                //     // Right channel
                //     sum += dexedPluginBuffers[i].getSample(channel, sample) * pan;
                // }

                // Do the same but don't apply panning fully, only apply it by panSpread %



                // if (channel == 0) {
                //     // Left channel
                //     sum += dexedPluginBuffers[i].getSample(channel, sample) * (1.0 - panAmountFactor * pan);
                // }
                // else {
                //     // Right channel
                //     sum += dexedPluginBuffers[i].getSample(channel, sample) * (panAmountFactor * pan + (1.0 - panAmountFactor));
                // }

                // Normalization factor, taking into account the number of unmuted instances and the pan amount factor
                double normalizationFactor = 1.0 / (numberOfUnmutedInstances * (1.0 - panAmountFactor * pan) + numberOfUnmutedInstances * (panAmountFactor * pan + (1.0 - panAmountFactor)));

                // Do the above but also apply normalization
                if (channel == 0) {
                    // Left channel
                    sum += dexedPluginBuffers[i].getSample(channel, sample) * (1.0 - panAmountFactor * pan) * normalizationFactor;
                }
                else {
                    // Right channel
                    sum += dexedPluginBuffers[i].getSample(channel, sample) * (panAmountFactor * pan + (1.0 - panAmountFactor)) * normalizationFactor;
                }

                // FIXME: Stereo not centered when panSpread is > 0.0
                // Something must be wrong because when one increases the spread, the stereo is no longer balanced
                // Maybe something is not linear?
                // What do we need to change so that the stereo is balanced when the spread is 0.0 and when the spread is 1.0
                // and for all values in between?

            }



            // buffer.setSample(channel, sample, sum / numberOfUnmutedstances);
            buffer.setSample(channel, sample, sum);

        }
    }
}

juce::AudioProcessorEditor *PluginAudioProcessor::createEditor()
{

    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return nullptr;
        }
    }

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
    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return;
        }
    }

    // Update the program in instance 0, the other instances will follow
    dexedPluginInstances[0]->setCurrentProgram(index);
}

const juce::String PluginAudioProcessor::getProgramName(int index)
{
    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return "";
        }
    }

    return dexedPluginInstances[0]->getProgramName(index);
}

void PluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {
    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return;
        }
    }

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
    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return false;
        }
    }

    return (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

// // Because we inherit from juce::AudioProcessorValueTreeState::Listener, we need to implement this method
void PluginAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    DBG("parameterChanged() called with parameterID = " + parameterID + " and newValue = " + juce::String(newValue));
    // If the parameterID is "detuneSpread", then we need to call detune()
    if (parameterID == "detuneSpread") {
        shouldSynchronize = false;
        detune();
        shouldSynchronize = true;
    }
}	

// Because we inherit from juce::AudioProcessorParameter::Listener, we need to implement this method
void PluginAudioProcessor::parameterValueChanged(int parameterIndex, float newValue) // We can't know which set of parameters the index refers to; FIXME
{
    // Return if any of the plugin instances are null
    for (int i = 0; i < numberOfInstances; i++) {
        if (dexedPluginInstances[i] == nullptr) {
            return;
        }
    }
    
    // Get the parameter that changed
    juce::AudioProcessorParameter* parameter = nullptr;
    for (int i = 0; i < dexedPluginInstances[0]->getParameters().size(); i++) {
        if (dexedPluginInstances[0]->getParameters()[i]->getParameterIndex() == parameterIndex) {
            parameter = dexedPluginInstances[0]->getParameters()[i];
            break;
        }
    }

    if (parameter == nullptr) {
        // Parameter not found, return
        return;
    }

    // Get the name of the parameter
    juce::String parameterName = parameter->getName(100);

    // We cannot distinguish between changes in the plugin instance and changes in the host,
    // so for now we just call detune() whenever the parameter changes, even if the user
    // changed the parameter with the same index in plugin instance 0 rather than the host

    std::cout << "Parameter " << parameterIndex << ": " << parameterName.toStdString() << " = " << newValue << std::endl;

    // Update the value of the parameter in all other plugin instances
    for (int i = 1; i < numberOfInstances; i++) {
        if (shouldSynchronize) {
            for (int j = 0; j < dexedPluginInstances[i]->getParameters().size(); j++) {
                if (dexedPluginInstances[i]->getParameters()[j]->getParameterIndex() == parameterIndex) {
                    dexedPluginInstances[i]->getParameters()[j]->setValueNotifyingHost(newValue);
                    break;
                }
            }
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
            dexedPluginInstances[i]->setStateInformation(state.getData(), static_cast<size_t>(state.getSize()));
        }
        detune();
        // Update the names of all programs exposed by the plugin to the host
        updateHostDisplay(); // TODO: Why does this not work? How can we update the menu containing the progams in the host?
        // dexedPluginInstances[0]->updateHostDisplay(); // Does not work either

        // Change the program to the one selected in instance 0
        setCurrentProgram(dexedPluginInstances[0]->getCurrentProgram());
    }
}

// Because we inherit from juce::AudioProcessorParameter::Listener, we need to implement this method
void PluginAudioProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    // Not used
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginAudioProcessor::createParameterLayout(){
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("detuneSpread", // parameterID
                                                        "Detune Spread", // parameter name
                                                        0.0f,   // minimum value
                                                        0.4f,   // maximum value
                                                        0.1f)); // default value
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("panSpread", // parameterID
                                                        "Pan Spread", // parameter name
                                                        0.0f,   // minimum value
                                                        1.0f,   // maximum value
                                                        1.0f)); // default value
   return { parameters.begin(), parameters.end() };               
}
