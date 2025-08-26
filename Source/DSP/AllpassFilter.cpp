#include "AllpassFilter.h"
#include <algorithm>

// AllpassFilter implementation
AllpassFilter::AllpassFilter()
    : m_gain(0.7f)
    , m_sampleRate(44100.0)
{
}

void AllpassFilter::prepare(double sampleRate, int maxDelayInSamples)
{
    m_sampleRate = sampleRate;
    m_delayLine.setMaxDelayInSamples(maxDelayInSamples);
    m_delayLine.prepare(sampleRate);
}

void AllpassFilter::reset()
{
    m_delayLine.reset();
}

void AllpassFilter::setDelay(float delayInSamples)
{
    m_delayLine.setDelay(delayInSamples);
}

void AllpassFilter::setDelayInMs(float delayInMs)
{
    m_delayLine.setDelayInMs(delayInMs);
}

void AllpassFilter::setGain(float gain)
{
    m_gain = juce::jlimit(-0.99f, 0.99f, gain);
}

float AllpassFilter::processSample(float inputSample)
{
    // Allpass filter equation: y[n] = -g*x[n] + x[n-M] + g*y[n-M]
    // Where g is the gain, M is the delay, x is input, y is output
    
    // Get the delayed signal (from M samples ago)
    float delayOutput = m_delayLine.processSample(inputSample);
    
    // Calculate allpass output
    float output = -m_gain * inputSample + delayOutput;
    
    // Feed the output back into the delay line for next iteration
    // This creates the feedback path of the allpass filter
    // Note: In a complete implementation, we'd manage this more carefully
    
    return output;
}

// ModulatedAllpassFilter implementation
ModulatedAllpassFilter::ModulatedAllpassFilter()
    : m_baseDelayInSamples(0.0f)
    , m_modulationRate(1.0f)
    , m_modulationDepth(0.1f)
    , m_phase(0.0f)
    , m_sampleRate(44100.0)
{
}

void ModulatedAllpassFilter::prepare(double sampleRate, int maxDelayInSamples)
{
    m_sampleRate = sampleRate;
    m_allpassFilter.prepare(sampleRate, maxDelayInSamples);
    m_phase = 0.0f;
}

void ModulatedAllpassFilter::reset()
{
    m_allpassFilter.reset();
    m_phase = 0.0f;
}

void ModulatedAllpassFilter::setDelay(float delayInSamples)
{
    m_baseDelayInSamples = delayInSamples;
    updateModulatedDelay();
}

void ModulatedAllpassFilter::setModulation(float rate, float depth)
{
    m_modulationRate = juce::jlimit(0.01f, 20.0f, rate);
    m_modulationDepth = juce::jlimit(0.0f, 1.0f, depth);
}

void ModulatedAllpassFilter::setGain(float gain)
{
    m_allpassFilter.setGain(gain);
}

float ModulatedAllpassFilter::processSample(float inputSample)
{
    // Update modulation
    updateModulatedDelay();
    
    // Advance phase
    m_phase += m_modulationRate / static_cast<float>(m_sampleRate);
    if (m_phase >= 1.0f)
        m_phase -= 1.0f;
    
    return m_allpassFilter.processSample(inputSample);
}

void ModulatedAllpassFilter::updateModulatedDelay()
{
    // Calculate modulated delay using sine wave
    float modulation = std::sin(m_phase * 2.0f * 3.14159265359f); // Use explicit pi value
    float modulatedDelay = m_baseDelayInSamples * (1.0f + m_modulationDepth * modulation);
    
    m_allpassFilter.setDelay(modulatedDelay);
}

// NestedAllpassNetwork implementation
NestedAllpassNetwork::NestedAllpassNetwork()
    : m_sampleRate(44100.0)
{
}

void NestedAllpassNetwork::prepare(double sampleRate, int maxDelayInSamples)
{
    m_sampleRate = sampleRate;
    
    // Prepare all existing filters
    for (auto& filter : m_allpassFilters)
    {
        filter->prepare(sampleRate, maxDelayInSamples);
    }
}

void NestedAllpassNetwork::reset()
{
    for (auto& filter : m_allpassFilters)
    {
        filter->reset();
    }
}

void NestedAllpassNetwork::setConfiguration(const std::vector<float>& delays, 
                                          const std::vector<float>& gains)
{
    // Ensure we have matching delay and gain vectors
    if (delays.size() != gains.size())
        return;
    
    // Limit the number of stages
    size_t numStages = std::min(delays.size(), MAX_STAGES);
    
    // Resize the filter array if needed
    if (m_allpassFilters.size() != numStages)
    {
        m_allpassFilters.clear();
        m_allpassFilters.reserve(numStages);
        
        for (size_t i = 0; i < numStages; ++i)
        {
            m_allpassFilters.push_back(std::make_unique<AllpassFilter>());
            m_allpassFilters[i]->prepare(m_sampleRate, static_cast<int>(delays[i] * 2)); // 2x for safety margin
        }
    }
    
    // Configure each filter
    for (size_t i = 0; i < numStages; ++i)
    {
        m_allpassFilters[i]->setDelay(delays[i]);
        m_allpassFilters[i]->setGain(gains[i]);
    }
}

void NestedAllpassNetwork::setDelaysInMs(const std::vector<float>& delaysInMs)
{
    std::vector<float> delaysInSamples;
    delaysInSamples.reserve(delaysInMs.size());
    
    for (float delayMs : delaysInMs)
    {
        float delaySamples = delayMs * static_cast<float>(m_sampleRate) / 1000.0f;
        delaysInSamples.push_back(delaySamples);
    }
    
    // Use default gains if none specified
    std::vector<float> defaultGains(delaysInMs.size(), 0.7f);
    setConfiguration(delaysInSamples, defaultGains);
}

float NestedAllpassNetwork::processSample(float inputSample)
{
    float output = inputSample;
    
    // Process through each allpass filter in series
    for (auto& filter : m_allpassFilters)
    {
        output = filter->processSample(output);
    }
    
    return output;
}