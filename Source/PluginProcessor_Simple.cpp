#include "PluginProcessor.h"
#include "PluginEditor.h"

// ReverbPluginAudioProcessor implementation
ReverbPluginAudioProcessor::ReverbPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor(BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput("Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , m_valueTreeState(*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize parameter references for real-time access
    initializeParameterReferences();
}

ReverbPluginAudioProcessor::~ReverbPluginAudioProcessor()
{
}

const juce::String ReverbPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReverbPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReverbPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReverbPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReverbPluginAudioProcessor::getTailLengthSeconds() const
{
    return static_cast<double>(m_reverbEngine.getDecayTime());
}

int ReverbPluginAudioProcessor::getNumPrograms()
{
    return 1; // Just default program for now
}

int ReverbPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReverbPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String ReverbPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return "Default";
}

void ReverbPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void ReverbPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_sampleRate = sampleRate;
    m_samplesPerBlock = samplesPerBlock;
    
    // Prepare reverb engine
    m_reverbEngine.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    
    // Update parameters from current state
    updateReverbParameters();
    
    m_isPrepared = true;
}

void ReverbPluginAudioProcessor::releaseResources()
{
    m_reverbEngine.reset();
    m_isPrepared = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
  #else
    // This plugin supports mono and stereo configurations
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    auto channelSet = layouts.getMainInputChannelSet();
    if (channelSet != juce::AudioChannelSet::mono() && 
        channelSet != juce::AudioChannelSet::stereo())
        return false;

    return true;
  #endif
}
#endif

void ReverbPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    
    if (!m_isPrepared)
        return;
    
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Update parameters if they've changed
    updateReverbParameters();
    
    // Process audio through reverb engine
    m_reverbEngine.processBlock(buffer);
}

bool ReverbPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ReverbPluginAudioProcessor::createEditor()
{
    return new ReverbPluginAudioProcessorEditor(*this);
}

void ReverbPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Create XML representation of the current state
    auto xml = std::make_unique<juce::XmlElement>("ReverbPluginState");
    
    // Save parameter values
    auto parametersXml = m_valueTreeState.copyState().createXml();
    if (parametersXml != nullptr)
        xml->addChildElement(parametersXml.release());
    
    // Convert to binary
    copyXmlToBinary(*xml, destData);
}

void ReverbPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Parse XML from binary data
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr && xmlState->hasTagName("ReverbPluginState"))
    {
        // Restore parameter values
        auto parametersXml = xmlState->getChildByName(m_valueTreeState.state.getType());
        if (parametersXml != nullptr)
        {
            m_valueTreeState.replaceState(juce::ValueTree::fromXml(*parametersXml));
        }
        
        // Update reverb engine with restored parameters
        updateReverbParameters();
    }
}

void ReverbPluginAudioProcessor::updateReverbParameters()
{
    if (!m_isPrepared)
        return;
    
    // Read atomic parameter values and update reverb engine
    if (m_roomSizeParam != nullptr)
        m_reverbEngine.setRoomSize(m_roomSizeParam->load());
    
    if (m_decayTimeParam != nullptr)
        m_reverbEngine.setDecayTime(m_decayTimeParam->load());
    
    if (m_preDelayParam != nullptr)
        m_reverbEngine.setPreDelay(m_preDelayParam->load());
    
    if (m_dampingParam != nullptr)
        m_reverbEngine.setDamping(m_dampingParam->load());
    
    if (m_wetDryMixParam != nullptr)
        m_reverbEngine.setWetDryMix(m_wetDryMixParam->load());
}

void ReverbPluginAudioProcessor::initializeParameterReferences()
{
    // Get atomic parameter references for real-time access
    m_roomSizeParam = m_valueTreeState.getRawParameterValue("roomSize");
    m_decayTimeParam = m_valueTreeState.getRawParameterValue("decayTime");
    m_preDelayParam = m_valueTreeState.getRawParameterValue("preDelay");
    m_dampingParam = m_valueTreeState.getRawParameterValue("damping");
    m_wetDryMixParam = m_valueTreeState.getRawParameterValue("wetDryMix");
}

juce::AudioProcessorValueTreeState::ParameterLayout ReverbPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Room Size parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("roomSize", 1),
        "Room Size",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f),
        1.0f
    ));
    
    // Decay Time parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("decayTime", 1),
        "Decay Time",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f),
        2.0f
    ));
    
    // Pre-Delay parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("preDelay", 1),
        "Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 500.0f, 1.0f),
        20.0f
    ));
    
    // Damping parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("damping", 1),
        "Damping",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f
    ));
    
    // Wet/Dry Mix parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("wetDryMix", 1),
        "Wet/Dry Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        30.0f
    ));
    
    return { parameters.begin(), parameters.end() };
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbPluginAudioProcessor();
}