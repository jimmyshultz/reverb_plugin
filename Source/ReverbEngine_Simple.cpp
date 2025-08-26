#include "ReverbEngine_Simple.h"
#include <cmath>
#include <algorithm>

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
}

void ReverbEngine::prepare(double sampleRate, int expectedBlockSize, int numChannels)
{
    m_sampleRate = sampleRate;
    m_numChannels = numChannels;
    
    // Setup pre-delay (max 500ms)
    int maxPreDelaySamples = static_cast<int>(0.5 * sampleRate);
    m_preDelay.setSize(maxPreDelaySamples);
    
    // Setup comb filters (4 parallel comb filters with different delays)
    m_combFilters.resize(4);
    std::vector<int> combDelays = {
        static_cast<int>(0.030 * sampleRate), // 30ms
        static_cast<int>(0.037 * sampleRate), // 37ms
        static_cast<int>(0.041 * sampleRate), // 41ms
        static_cast<int>(0.044 * sampleRate)  // 44ms
    };
    
    for (size_t i = 0; i < m_combFilters.size(); ++i)
    {
        m_combFilters[i].setSize(combDelays[i]);
    }
    
    // Setup early reflections (8 taps)
    m_earlyDelays.resize(8);
    m_earlyGains.resize(8);
    std::vector<int> earlyDelays = {
        static_cast<int>(0.008 * sampleRate), // 8ms
        static_cast<int>(0.012 * sampleRate), // 12ms
        static_cast<int>(0.016 * sampleRate), // 16ms
        static_cast<int>(0.020 * sampleRate), // 20ms
        static_cast<int>(0.024 * sampleRate), // 24ms
        static_cast<int>(0.028 * sampleRate), // 28ms
        static_cast<int>(0.035 * sampleRate), // 35ms
        static_cast<int>(0.042 * sampleRate)  // 42ms
    };
    
    for (size_t i = 0; i < m_earlyDelays.size(); ++i)
    {
        m_earlyDelays[i].setSize(earlyDelays[i]);
        m_earlyGains[i] = 0.6f * std::pow(0.8f, static_cast<float>(i)); // Decreasing gains
    }
    
    updateParameters();
    m_isPrepared = true;
}

void ReverbEngine::reset()
{
    if (!m_isPrepared) return;
    
    m_preDelay.clear();
    for (auto& comb : m_combFilters)
        comb.clear();
    for (auto& early : m_earlyDelays)
        early.clear();
}

void ReverbEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (!m_isPrepared) return;
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
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
    // Store dry signals
    float dryLeft = leftInput;
    float dryRight = rightInput;
    
    // Mix to mono for reverb processing
    float monoInput = (leftInput + rightInput) * 0.5f;
    
    // Apply pre-delay
    float preDelayed = m_preDelay.process(monoInput);
    
    // Process early reflections
    float earlyReflections = 0.0f;
    for (size_t i = 0; i < m_earlyDelays.size(); ++i)
    {
        earlyReflections += m_earlyDelays[i].process(preDelayed) * m_earlyGains[i];
    }
    earlyReflections *= 0.25f; // Scale down
    
    // Process late reverb (comb filters)
    float lateReverb = 0.0f;
    for (auto& comb : m_combFilters)
    {
        lateReverb += comb.process(preDelayed);
    }
    lateReverb *= 0.25f; // Mix down the 4 comb filters
    
    // Balance early and late reflections
    float earlyGain = m_earlyLateBalance / 100.0f;
    float lateGain = 1.0f - earlyGain;
    float reverb = earlyReflections * earlyGain + lateReverb * lateGain;
    
    // Apply wet/dry mixing
    float wetGain = m_wetDryMix / 100.0f;
    float dryGain = 1.0f - wetGain;
    
    leftOutput = dryLeft * dryGain + reverb * wetGain;
    rightOutput = dryRight * dryGain + reverb * wetGain * 0.95f; // Slight stereo decorrelation
}

void ReverbEngine::updateParameters()
{
    if (!m_isPrepared) return;
    
    // Update pre-delay
    int preDelaySamples = static_cast<int>(m_preDelayMs * m_sampleRate / 1000.0f);
    // Note: In a more complete implementation, we'd smoothly transition the delay time
    
    // Update comb filter feedback based on decay time
    for (auto& comb : m_combFilters)
    {
        // Calculate feedback for desired decay time
        float feedback = std::pow(0.001f, 1.0f / (m_decayTime * m_sampleRate / 30.0f)); // Approximate
        feedback = juce::jlimit(0.0f, 0.95f, feedback);
        comb.setFeedback(feedback);
    }
}

// Parameter setters
void ReverbEngine::setRoomSize(float roomSize)
{
    m_roomSize = juce::jlimit(0.1f, 10.0f, roomSize);
    updateParameters();
}

void ReverbEngine::setDecayTime(float decayTime)
{
    m_decayTime = juce::jlimit(0.1f, 20.0f, decayTime);
    updateParameters();
}

void ReverbEngine::setPreDelay(float preDelayMs)
{
    m_preDelayMs = juce::jlimit(0.0f, 500.0f, preDelayMs);
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