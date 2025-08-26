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
    , m_valueTreeState(*this, nullptr, "Parameters", ReverbParams::createParameterLayout())
    , m_presetManager(m_valueTreeState)
{
    // Initialize parameter references for real-time access
    initializeParameterReferences();
    
    // Initialize level smoothers
    m_inputLevelSmoother.setCurrentAndTargetValue(0.0f);
    m_outputLevelSmoother.setCurrentAndTargetValue(0.0f);
    
    // Set initial CPU measurement time
    m_lastCPUMeasurement = juce::Time::getCurrentTime();
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
    // Return the reverb tail length for proper plugin behavior
    return static_cast<double>(m_reverbEngine.getDecayTime());
}

int ReverbPluginAudioProcessor::getNumPrograms()
{
    return m_presetManager.getNumPresets();
}

int ReverbPluginAudioProcessor::getCurrentProgram()
{
    return m_presetManager.getCurrentPresetIndex();
}

void ReverbPluginAudioProcessor::setCurrentProgram(int index)
{
    m_presetManager.loadPreset(index);
}

const juce::String ReverbPluginAudioProcessor::getProgramName(int index)
{
    return m_presetManager.getPresetName(index);
}

void ReverbPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    m_presetManager.renamePreset(index, newName);
}

void ReverbPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_sampleRate = sampleRate;
    m_samplesPerBlock = samplesPerBlock;
    
    // Prepare reverb engine
    m_reverbEngine.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
    
    // Prepare dry/wet mixer for bypass functionality
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    m_dryWetMixer.prepare(spec);
    m_dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    
    // Prepare level smoothers
    m_inputLevelSmoother.reset(sampleRate, 0.1); // 100ms smoothing
    m_outputLevelSmoother.reset(sampleRate, 0.1);
    
    // Update parameters from current state
    updateReverbParameters();
    
    m_isPrepared = true;
}

void ReverbPluginAudioProcessor::releaseResources()
{
    m_reverbEngine.reset();
    m_dryWetMixer.reset();
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
    
    auto startTime = juce::Time::getCurrentTime();
    
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Update parameters if they've changed
    updateReverbParameters();
    
    // Handle bypass state
    bool currentlyBypassed = isSuspended() || !m_isPrepared;
    
    if (currentlyBypassed != m_wasBypassed)
    {
        m_dryWetMixer.setWetMixProportion(currentlyBypassed ? 0.0f : 1.0f);
        m_wasBypassed = currentlyBypassed;
    }
    
    // Store dry signal for bypass mixing
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    
    // Process audio through reverb engine
    if (!currentlyBypassed)
    {
        m_reverbEngine.processBlock(buffer);
    }
    
    // Apply dry/wet mixing for bypass
    juce::dsp::AudioBlock<float> dryBlock(dryBuffer);
    juce::dsp::AudioBlock<float> wetBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(wetBlock);
    
    m_dryWetMixer.pushDrySamples(dryBlock);
    m_dryWetMixer.mixWetSamples(wetBlock);
    
    // Update performance metrics and level meters
    updateLevelMeters(buffer);
    updatePerformanceMetrics(startTime);
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
    
    // Save preset information
    auto presetXml = std::make_unique<juce::XmlElement>("CurrentPreset");
    presetXml->setAttribute("index", m_presetManager.getCurrentPresetIndex());
    xml->addChildElement(presetXml.release());
    
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
        
        // Restore preset information
        auto presetXml = xmlState->getChildByName("CurrentPreset");
        if (presetXml != nullptr)
        {
            int presetIndex = presetXml->getIntAttribute("index", -1);
            if (presetIndex >= 0 && presetIndex < m_presetManager.getNumPresets())
            {
                m_presetManager.loadPreset(presetIndex);
            }
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
    
    if (m_earlyLateBalanceParam != nullptr)
        m_reverbEngine.setEarlyLateBalance(m_earlyLateBalanceParam->load());
    
    if (m_diffusionParam != nullptr)
        m_reverbEngine.setDiffusion(m_diffusionParam->load());
    
    if (m_modulationRateParam != nullptr)
        m_reverbEngine.setModulationRate(m_modulationRateParam->load());
    
    if (m_modulationDepthParam != nullptr)
        m_reverbEngine.setModulationDepth(m_modulationDepthParam->load());
}

void ReverbPluginAudioProcessor::updatePerformanceMetrics(juce::Time startTime)
{
    auto currentTime = juce::Time::getCurrentTime();
    auto processingTime = currentTime - startTime;
    auto timeSinceLastMeasurement = currentTime - m_lastCPUMeasurement;
    
    // Update CPU usage every 100ms
    if (timeSinceLastMeasurement.inMilliseconds() >= 100)
    {
        auto blockDuration = m_samplesPerBlock / m_sampleRate;
        m_cpuUsage = static_cast<float>(processingTime.inSeconds() / blockDuration) * 100.0f;
        m_lastCPUMeasurement = currentTime;
    }
}

void ReverbPluginAudioProcessor::updateLevelMeters(const juce::AudioBuffer<float>& buffer)
{
    // Calculate input and output peak levels
    float inputPeak = 0.0f;
    float outputPeak = 0.0f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto channelData = buffer.getReadPointer(channel);
        auto channelPeak = juce::FloatVectorOperations::findMaximum(channelData, buffer.getNumSamples());
        inputPeak = std::max(inputPeak, std::abs(channelPeak));
        outputPeak = std::max(outputPeak, std::abs(channelPeak));
    }
    
    // Smooth the level values
    m_inputLevelSmoother.setTargetValue(inputPeak);
    m_outputLevelSmoother.setTargetValue(outputPeak);
    
    // Update atomic values for thread-safe access from GUI
    m_peakInputLevel.store(m_inputLevelSmoother.getNextValue());
    m_peakOutputLevel.store(m_outputLevelSmoother.getNextValue());
}

void ReverbPluginAudioProcessor::initializeParameterReferences()
{
    // Get atomic parameter references for real-time access
    m_roomSizeParam = m_valueTreeState.getRawParameterValue(ReverbParams::ROOM_SIZE_ID);
    m_decayTimeParam = m_valueTreeState.getRawParameterValue(ReverbParams::DECAY_TIME_ID);
    m_preDelayParam = m_valueTreeState.getRawParameterValue(ReverbParams::PRE_DELAY_ID);
    m_dampingParam = m_valueTreeState.getRawParameterValue(ReverbParams::DAMPING_ID);
    m_wetDryMixParam = m_valueTreeState.getRawParameterValue(ReverbParams::WET_DRY_MIX_ID);
    m_earlyLateBalanceParam = m_valueTreeState.getRawParameterValue(ReverbParams::EARLY_LATE_BALANCE_ID);
    m_diffusionParam = m_valueTreeState.getRawParameterValue(ReverbParams::DIFFUSION_ID);
    m_modulationRateParam = m_valueTreeState.getRawParameterValue(ReverbParams::MODULATION_RATE_ID);
    m_modulationDepthParam = m_valueTreeState.getRawParameterValue(ReverbParams::MODULATION_DEPTH_ID);
}

// ReverbParameterListener implementation
ReverbParameterListener::ReverbParameterListener(ReverbPluginAudioProcessor& processor)
    : m_processor(processor)
{
    // Add listener to all parameters
    auto& apvts = m_processor.getValueTreeState();
    
    apvts.addParameterListener(ReverbParams::ROOM_SIZE_ID, this);
    apvts.addParameterListener(ReverbParams::DECAY_TIME_ID, this);
    apvts.addParameterListener(ReverbParams::PRE_DELAY_ID, this);
    apvts.addParameterListener(ReverbParams::DAMPING_ID, this);
    apvts.addParameterListener(ReverbParams::WET_DRY_MIX_ID, this);
    apvts.addParameterListener(ReverbParams::EARLY_LATE_BALANCE_ID, this);
    apvts.addParameterListener(ReverbParams::DIFFUSION_ID, this);
    apvts.addParameterListener(ReverbParams::MODULATION_RATE_ID, this);
    apvts.addParameterListener(ReverbParams::MODULATION_DEPTH_ID, this);
}

ReverbParameterListener::~ReverbParameterListener()
{
    // Remove listener from all parameters
    auto& apvts = m_processor.getValueTreeState();
    
    apvts.removeParameterListener(ReverbParams::ROOM_SIZE_ID, this);
    apvts.removeParameterListener(ReverbParams::DECAY_TIME_ID, this);
    apvts.removeParameterListener(ReverbParams::PRE_DELAY_ID, this);
    apvts.removeParameterListener(ReverbParams::DAMPING_ID, this);
    apvts.removeParameterListener(ReverbParams::WET_DRY_MIX_ID, this);
    apvts.removeParameterListener(ReverbParams::EARLY_LATE_BALANCE_ID, this);
    apvts.removeParameterListener(ReverbParams::DIFFUSION_ID, this);
    apvts.removeParameterListener(ReverbParams::MODULATION_RATE_ID, this);
    apvts.removeParameterListener(ReverbParams::MODULATION_DEPTH_ID, this);
}

void ReverbParameterListener::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Handle real-time parameter changes
    juce::ignoreUnused(parameterID, newValue);
    
    // The processor will handle parameter updates in its audio thread
    // This could be used for additional processing or notifications
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbPluginAudioProcessor();
}