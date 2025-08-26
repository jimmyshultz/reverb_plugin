#include "ReverbEngine.h"
#include <cmath>
#include <algorithm>

// ReverbEngine implementation
ReverbEngine::ReverbEngine()
    : m_roomSize(1.0f)
    , m_decayTime(2.0f)
    , m_preDelayMs(20.0f)
    , m_damping(50.0f)
    , m_wetDryMix(30.0f)
    , m_earlyLateBalance(50.0f)
    , m_diffusion(75.0f)
    , m_modulationRate(1.0f)
    , m_modulationDepth(25.0f)
    , m_sampleRate(44100.0)
    , m_numChannels(2)
    , m_isPrepared(false)
{
    initializeComponents();
}

void ReverbEngine::prepare(double sampleRate, int expectedBlockSize, int numChannels)
{
    m_sampleRate = sampleRate;
    m_numChannels = numChannels;
    
    // Calculate maximum delay times based on parameters
    int maxPreDelaySamples = static_cast<int>(MAX_PRE_DELAY_MS * sampleRate / 1000.0);
    int maxCombDelaySamples = static_cast<int>(BASE_COMB_DELAY_MS * MAX_ROOM_SIZE * sampleRate / 1000.0);
    int maxAllpassDelaySamples = static_cast<int>(BASE_ALLPASS_DELAY_MS * MAX_ROOM_SIZE * sampleRate / 1000.0);
    int maxEarlyReflectionDelay = static_cast<int>(100.0 * sampleRate / 1000.0); // 100ms max
    
    // Prepare all components
    m_preDelayLeft->prepare(sampleRate);
    m_preDelayLeft->setMaxDelayInSamples(maxPreDelaySamples);
    
    if (numChannels > 1)
    {
        m_preDelayRight->prepare(sampleRate);
        m_preDelayRight->setMaxDelayInSamples(maxPreDelaySamples);
    }
    
    m_earlyReflections->prepare(sampleRate);
    m_earlyReflections->setMaxDelayInSamples(maxEarlyReflectionDelay);
    
    m_combFilterBank->prepare(sampleRate, maxCombDelaySamples);
    m_allpassNetwork->prepare(sampleRate, maxAllpassDelaySamples);
    m_modulationMatrix->prepare(sampleRate);
    
    // Configure components with current parameters
    configureEarlyReflections();
    configureCombFilters();
    configureAllpassFilters();
    configureModulation();
    
    updateParameters();
    
    m_isPrepared = true;
}

void ReverbEngine::reset()
{
    if (!m_isPrepared)
        return;
    
    m_preDelayLeft->reset();
    if (m_numChannels > 1 && m_preDelayRight)
        m_preDelayRight->reset();
    
    m_earlyReflections->reset();
    m_combFilterBank->reset();
    m_allpassNetwork->reset();
    m_modulationMatrix->reset();
}

void ReverbEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!m_isPrepared)
        return;
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Update modulation matrix
    for (int sample = 0; sample < numSamples; ++sample)
    {
        m_modulationMatrix->processSample();
        
        if (numChannels == 1)
        {
            // Mono processing
            float input = buffer.getSample(0, sample);
            float leftOutput, rightOutput;
            processStereo(input, input, leftOutput, rightOutput);
            buffer.setSample(0, sample, (leftOutput + rightOutput) * 0.5f);
        }
        else
        {
            // Stereo processing
            float leftInput = buffer.getSample(0, sample);
            float rightInput = buffer.getSample(1, sample);
            float leftOutput, rightOutput;
            
            processStereo(leftInput, rightInput, leftOutput, rightOutput);
            
            buffer.setSample(0, sample, leftOutput);
            buffer.setSample(1, sample, rightOutput);
        }
    }
}

void ReverbEngine::processStereo(float leftInput, float rightInput,
                                float& leftOutput, float& rightOutput)
{
    // Store dry signals for mixing
    float dryLeft = leftInput;
    float dryRight = rightInput;
    
    // Apply pre-delay
    float preDelayedLeft = m_preDelayLeft->processSample(leftInput);
    float preDelayedRight = (m_numChannels > 1 && m_preDelayRight) ? 
                           m_preDelayRight->processSample(rightInput) : preDelayedLeft;
    
    // Process early reflections
    float earlyLeft, earlyRight;
    processEarlyReflections(preDelayedLeft, preDelayedRight, earlyLeft, earlyRight);
    
    // Mix to mono for late reverb processing
    float monoInput = (preDelayedLeft + preDelayedRight) * 0.5f;
    
    // Process late reverb
    float lateReverb = processLateReverb(monoInput);
    
    // Create stereo late reverb with slight decorrelation
    float lateLeft = lateReverb;
    float lateRight = lateReverb * 0.95f; // Slight attenuation for stereo effect
    
    // Balance early and late reflections
    float earlyGain = m_earlyLateBalance / 100.0f;
    float lateGain = 1.0f - earlyGain;
    
    float wetLeft = earlyLeft * earlyGain + lateLeft * lateGain;
    float wetRight = earlyRight * earlyGain + lateRight * lateGain;
    
    // Apply wet/dry mixing
    leftOutput = applyWetDryMix(dryLeft, wetLeft);
    rightOutput = applyWetDryMix(dryRight, wetRight);
    
    // Apply stereo width based on diffusion parameter
    float stereoWidth = m_diffusion / 100.0f;
    applyStereoWidth(leftOutput, rightOutput, leftOutput, rightOutput, stereoWidth);
}

void ReverbEngine::initializeComponents()
{
    m_preDelayLeft = std::make_unique<DelayLine>();
    m_preDelayRight = std::make_unique<DelayLine>();
    m_earlyReflections = std::make_unique<MultiTapDelayLine>();
    m_combFilterBank = std::make_unique<ParallelCombFilterBank>();
    m_allpassNetwork = std::make_unique<NestedAllpassNetwork>();
    m_modulationMatrix = std::make_unique<ModulationMatrix>();
}

void ReverbEngine::configureEarlyReflections()
{
    // Define early reflection pattern (typical values for a medium room)
    std::vector<MultiTapDelayLine::DelayTap> taps;
    
    // Early reflections from walls, ceiling, and floor
    taps.push_back({calculateRoomScaledDelay(8.0f), 0.6f, -0.7f});   // Left wall
    taps.push_back({calculateRoomScaledDelay(12.0f), 0.5f, 0.3f});   // Right wall  
    taps.push_back({calculateRoomScaledDelay(16.0f), 0.4f, 0.0f});   // Back wall
    taps.push_back({calculateRoomScaledDelay(20.0f), 0.35f, -0.5f}); // Ceiling
    taps.push_back({calculateRoomScaledDelay(24.0f), 0.3f, 0.6f});   // Floor
    taps.push_back({calculateRoomScaledDelay(28.0f), 0.25f, 0.8f});  // Corner
    taps.push_back({calculateRoomScaledDelay(35.0f), 0.2f, -0.4f});  // Complex reflection
    taps.push_back({calculateRoomScaledDelay(42.0f), 0.15f, 0.1f});  // Complex reflection
    
    m_earlyReflections->setTaps(taps);
}

void ReverbEngine::configureCombFilters()
{
    // Standard comb filter delays (in milliseconds) for Schroeder reverb
    // These values are chosen to avoid harmonic relationships
    std::vector<float> baseDelays = {29.7f, 37.1f, 41.1f, 43.7f};
    
    std::vector<float> scaledDelays;
    for (float delay : baseDelays)
    {
        scaledDelays.push_back(calculateRoomScaledDelay(delay));
    }
    
    // Convert to samples
    std::vector<float> delaysInSamples;
    for (float delayMs : scaledDelays)
    {
        delaysInSamples.push_back(delayMs * static_cast<float>(m_sampleRate) / 1000.0f);
    }
    
    // Set feedback gains based on decay time
    std::vector<float> feedbacks;
    for (size_t i = 0; i < delaysInSamples.size(); ++i)
    {
        float feedback = calculateDecayFeedback(m_decayTime);
        feedbacks.push_back(feedback);
    }
    
    // Output gains for mixing
    std::vector<float> gains(delaysInSamples.size(), 0.25f); // Equal mixing
    
    m_combFilterBank->setConfiguration(delaysInSamples, feedbacks, gains);
    m_combFilterBank->setGlobalDamping(m_damping / 100.0f);
}

void ReverbEngine::configureAllpassFilters()
{
    // Allpass filter delays for diffusion
    std::vector<float> baseDelays = {5.0f, 1.7f, 2.3f};
    
    std::vector<float> delaysInMs;
    for (float delay : baseDelays)
    {
        delaysInMs.push_back(calculateRoomScaledDelay(delay));
    }
    
    m_allpassNetwork->setDelaysInMs(delaysInMs);
    
    // Set gains based on diffusion parameter
    float diffusionGain = (m_diffusion / 100.0f) * 0.7f; // Max 0.7 for stability
    std::vector<float> gains(delaysInMs.size(), diffusionGain);
    
    std::vector<float> delaysInSamples;
    for (float delayMs : delaysInMs)
    {
        delaysInSamples.push_back(delayMs * static_cast<float>(m_sampleRate) / 1000.0f);
    }
    
    m_allpassNetwork->setConfiguration(delaysInSamples, gains);
}

void ReverbEngine::configureModulation()
{
    // Set up modulation routing
    ModulationMatrix::ModulationRoute route1;
    route1.source = ModulationMatrix::Source::LFO1;
    route1.destination = ModulationMatrix::Destination::DelayTime;
    route1.amount = m_modulationDepth / 100.0f * 0.1f; // Small modulation amount
    route1.enabled = true;
    m_modulationMatrix->setRoute(route1);
    
    ModulationMatrix::ModulationRoute route2;
    route2.source = ModulationMatrix::Source::LFO2;
    route2.destination = ModulationMatrix::Destination::Diffusion;
    route2.amount = m_modulationDepth / 100.0f * 0.05f;
    route2.enabled = true;
    m_modulationMatrix->setRoute(route2);
}

void ReverbEngine::updateParameters()
{
    if (!m_isPrepared)
        return;
    
    // Update pre-delay
    m_preDelayLeft->setDelayInMs(m_preDelayMs);
    if (m_numChannels > 1 && m_preDelayRight)
        m_preDelayRight->setDelayInMs(m_preDelayMs * 1.05f); // Slight stereo offset
    
    // Reconfigure components based on new parameters
    configureEarlyReflections();
    configureCombFilters(); 
    configureAllpassFilters();
    configureModulation();
}

float ReverbEngine::calculateRoomScaledDelay(float baseDelayMs) const
{
    return baseDelayMs * m_roomSize;
}

float ReverbEngine::calculateDecayFeedback(float decayTime) const
{
    // Calculate feedback for 60dB decay over the specified time
    // Feedback = 10^(-3 * delay_time / (decay_time * sample_rate))
    float delayTimeSeconds = BASE_COMB_DELAY_MS / 1000.0f;
    float feedback = std::pow(10.0f, -3.0f * delayTimeSeconds / decayTime);
    return juce::jlimit(0.0f, 0.95f, feedback);
}

void ReverbEngine::processEarlyReflections(float leftInput, float rightInput,
                                         float& leftOutput, float& rightOutput)
{
    m_earlyReflections->processStereo(leftInput, rightInput, leftOutput, rightOutput);
}

float ReverbEngine::processLateReverb(float input)
{
    // Process through comb filter bank
    float combOutput = m_combFilterBank->processSample(input);
    
    // Apply modulation to delay times
    float delayModulation = m_modulationMatrix->getModulationValue(ModulationMatrix::Destination::DelayTime);
    // Note: In a complete implementation, we would apply this modulation to individual delay lines
    
    // Process through allpass network for diffusion
    float diffusedOutput = m_allpassNetwork->processSample(combOutput);
    
    return diffusedOutput;
}

float ReverbEngine::applyWetDryMix(float drySignal, float wetSignal) const
{
    float wetGain = m_wetDryMix / 100.0f;
    float dryGain = 1.0f - wetGain;
    
    return drySignal * dryGain + wetSignal * wetGain;
}

void ReverbEngine::applyStereoWidth(float leftSignal, float rightSignal,
                                   float& leftOutput, float& rightOutput,
                                   float width) const
{
    // Stereo width processing using mid-side technique
    float mid = (leftSignal + rightSignal) * 0.5f;
    float side = (leftSignal - rightSignal) * 0.5f;
    
    // Apply width
    side *= width;
    
    // Convert back to left-right
    leftOutput = mid + side;
    rightOutput = mid - side;
}

// Parameter setters
void ReverbEngine::setRoomSize(float roomSize)
{
    m_roomSize = juce::jlimit(0.1f, MAX_ROOM_SIZE, roomSize);
    updateParameters();
}

void ReverbEngine::setDecayTime(float decayTime)
{
    m_decayTime = juce::jlimit(0.1f, MAX_DECAY_TIME, decayTime);
    updateParameters();
}

void ReverbEngine::setPreDelay(float preDelayMs)
{
    m_preDelayMs = juce::jlimit(0.0f, MAX_PRE_DELAY_MS, preDelayMs);
    updateParameters();
}

void ReverbEngine::setDamping(float damping)
{
    m_damping = juce::jlimit(0.0f, 100.0f, damping);
    updateParameters();
}

void ReverbEngine::setWetDryMix(float wetDryMix)
{
    m_wetDryMix = juce::jlimit(0.0f, 100.0f, wetDryMix);
}

void ReverbEngine::setEarlyLateBalance(float balance)
{
    m_earlyLateBalance = juce::jlimit(0.0f, 100.0f, balance);
}

void ReverbEngine::setDiffusion(float diffusion)
{
    m_diffusion = juce::jlimit(0.0f, 100.0f, diffusion);
    updateParameters();
}

void ReverbEngine::setModulationRate(float rate)
{
    m_modulationRate = juce::jlimit(0.1f, 5.0f, rate);
    updateParameters();
}

void ReverbEngine::setModulationDepth(float depth)
{
    m_modulationDepth = juce::jlimit(0.0f, 100.0f, depth);
    updateParameters();
}

// ReverbPreset implementation
std::vector<ReverbPreset::PresetData> ReverbPreset::s_factoryPresets;
bool ReverbPreset::s_presetsInitialized = false;

std::vector<ReverbPreset::PresetData> ReverbPreset::getFactoryPresets()
{
    if (!s_presetsInitialized)
    {
        initializeFactoryPresets();
        s_presetsInitialized = true;
    }
    
    return s_factoryPresets;
}

void ReverbPreset::applyPreset(ReverbEngine& engine, const PresetData& preset)
{
    engine.setRoomSize(preset.roomSize);
    engine.setDecayTime(preset.decayTime);
    engine.setPreDelay(preset.preDelay);
    engine.setDamping(preset.damping);
    engine.setWetDryMix(preset.wetDryMix);
    engine.setEarlyLateBalance(preset.earlyLateBalance);
    engine.setDiffusion(preset.diffusion);
    engine.setModulationRate(preset.modulationRate);
    engine.setModulationDepth(preset.modulationDepth);
}

ReverbPreset::PresetData ReverbPreset::createPresetFromEngine(const ReverbEngine& engine, const juce::String& name)
{
    PresetData preset;
    preset.name = name;
    preset.roomSize = engine.getRoomSize();
    preset.decayTime = engine.getDecayTime();
    preset.preDelay = engine.getPreDelay();
    preset.damping = engine.getDamping();
    preset.wetDryMix = engine.getWetDryMix();
    preset.earlyLateBalance = engine.getEarlyLateBalance();
    preset.diffusion = engine.getDiffusion();
    preset.modulationRate = engine.getModulationRate();
    preset.modulationDepth = engine.getModulationDepth();
    
    return preset;
}

void ReverbPreset::initializeFactoryPresets()
{
    s_factoryPresets.clear();
    
    // Small Room
    s_factoryPresets.push_back({
        "Small Room", 0.3f, 0.8f, 5.0f, 60.0f, 25.0f, 60.0f, 70.0f, 0.8f, 15.0f
    });
    
    // Medium Hall
    s_factoryPresets.push_back({
        "Medium Hall", 1.5f, 2.5f, 25.0f, 40.0f, 35.0f, 50.0f, 80.0f, 1.2f, 20.0f
    });
    
    // Large Cathedral
    s_factoryPresets.push_back({
        "Large Cathedral", 3.5f, 6.0f, 80.0f, 20.0f, 40.0f, 40.0f, 85.0f, 0.6f, 30.0f
    });
    
    // Plate Reverb
    s_factoryPresets.push_back({
        "Plate Reverb", 0.8f, 3.2f, 2.0f, 75.0f, 45.0f, 70.0f, 60.0f, 2.1f, 35.0f
    });
    
    // Spring Reverb
    s_factoryPresets.push_back({
        "Spring Reverb", 0.4f, 1.5f, 8.0f, 85.0f, 50.0f, 80.0f, 45.0f, 3.5f, 45.0f
    });
    
    // Ambient Space
    s_factoryPresets.push_back({
        "Ambient Space", 4.5f, 8.0f, 150.0f, 10.0f, 60.0f, 30.0f, 90.0f, 0.4f, 50.0f
    });
    
    // Vocal Reverb
    s_factoryPresets.push_back({
        "Vocal Reverb", 1.2f, 1.8f, 40.0f, 55.0f, 30.0f, 65.0f, 75.0f, 1.0f, 20.0f
    });
    
    // Drum Reverb
    s_factoryPresets.push_back({
        "Drum Reverb", 0.6f, 1.2f, 15.0f, 70.0f, 35.0f, 75.0f, 65.0f, 1.8f, 25.0f
    });
}