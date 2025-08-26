#include "CombFilter.h"
#include <algorithm>
#include <cmath>

// CombFilter implementation
CombFilter::CombFilter()
    : m_feedbackGain(0.7f)
    , m_feedforwardGain(1.0f)
    , m_sampleRate(44100.0)
    , m_lastOutput(0.0f)
{
}

void CombFilter::prepare(double sampleRate, int maxDelayInSamples)
{
    m_sampleRate = sampleRate;
    m_delayLine.setMaxDelayInSamples(maxDelayInSamples);
    m_delayLine.prepare(sampleRate);
}

void CombFilter::reset()
{
    m_delayLine.reset();
    m_lastOutput = 0.0f;
}

void CombFilter::setDelay(float delayInSamples)
{
    m_delayLine.setDelay(delayInSamples);
}

void CombFilter::setDelayInMs(float delayInMs)
{
    m_delayLine.setDelayInMs(delayInMs);
}

void CombFilter::setFeedback(float gain)
{
    m_feedbackGain = juce::jlimit(0.0f, 0.99f, gain);
}

void CombFilter::setFeedforward(float gain)
{
    m_feedforwardGain = juce::jlimit(0.0f, 2.0f, gain);
}

float CombFilter::processSample(float inputSample)
{
    // Comb filter equation: y[n] = x[n] + g * x[n-M]
    // For feedback comb: y[n] = x[n] + g * y[n-M]
    
    // Get the delayed output
    float delayedOutput = m_delayLine.processSample(inputSample + m_feedbackGain * m_lastOutput);
    
    // Calculate current output
    float output = m_feedforwardGain * inputSample + delayedOutput;
    
    // Store for next iteration
    m_lastOutput = output;
    
    return output;
}

// DampedCombFilter implementation
DampedCombFilter::DampedCombFilter()
    : m_sampleRate(44100.0)
{
}

void DampedCombFilter::prepare(double sampleRate, int maxDelayInSamples)
{
    m_sampleRate = sampleRate;
    m_combFilter.prepare(sampleRate, maxDelayInSamples);
    m_dampingFilter.prepare(sampleRate);
}

void DampedCombFilter::reset()
{
    m_combFilter.reset();
    m_dampingFilter.reset();
}

void DampedCombFilter::setDelay(float delayInSamples)
{
    m_combFilter.setDelay(delayInSamples);
}

void DampedCombFilter::setDelayInMs(float delayInMs)
{
    m_combFilter.setDelayInMs(delayInMs);
}

void DampedCombFilter::setFeedback(float gain)
{
    m_combFilter.setFeedback(gain);
}

void DampedCombFilter::setDamping(float damping)
{
    float cutoffFreq = dampingToCutoff(damping);
    m_dampingFilter.setCutoffFrequency(cutoffFreq);
}

float DampedCombFilter::processSample(float inputSample)
{
    // Process through comb filter first
    float combOutput = m_combFilter.processSample(inputSample);
    
    // Apply damping filter to the feedback path
    // Note: In a real implementation, the damping filter would be in the feedback loop
    // This is a simplified approach
    float dampedOutput = m_dampingFilter.processSample(combOutput);
    
    return dampedOutput;
}

float DampedCombFilter::dampingToCutoff(float damping) const
{
    // Map damping (0.0 to 1.0) to cutoff frequency
    // 0.0 damping = high cutoff (20kHz), 1.0 damping = low cutoff (100Hz)
    float minCutoff = 100.0f;
    float maxCutoff = 20000.0f;
    
    // Use exponential mapping for more natural response
    float normalizedCutoff = std::pow(1.0f - damping, 2.0f);
    return minCutoff + normalizedCutoff * (maxCutoff - minCutoff);
}

// ParallelCombFilterBank implementation
ParallelCombFilterBank::ParallelCombFilterBank()
    : m_sampleRate(44100.0)
{
}

void ParallelCombFilterBank::prepare(double sampleRate, int maxDelayInSamples)
{
    m_sampleRate = sampleRate;
    
    // Prepare all existing filters
    for (auto& filter : m_combFilters)
    {
        filter->prepare(sampleRate, maxDelayInSamples);
    }
}

void ParallelCombFilterBank::reset()
{
    for (auto& filter : m_combFilters)
    {
        filter->reset();
    }
}

void ParallelCombFilterBank::setConfiguration(const std::vector<float>& delays,
                                            const std::vector<float>& feedbacks,
                                            const std::vector<float>& gains)
{
    // Ensure all vectors have the same size
    if (delays.size() != feedbacks.size() || delays.size() != gains.size())
        return;
    
    size_t numFilters = std::min(delays.size(), MAX_COMB_FILTERS);
    
    // Resize filter arrays if needed
    if (m_combFilters.size() != numFilters)
    {
        m_combFilters.clear();
        m_outputGains.clear();
        
        m_combFilters.reserve(numFilters);
        m_outputGains.reserve(numFilters);
        
        for (size_t i = 0; i < numFilters; ++i)
        {
            m_combFilters.push_back(std::make_unique<DampedCombFilter>());
            m_combFilters[i]->prepare(m_sampleRate, static_cast<int>(delays[i] * 2)); // 2x for safety
            m_outputGains.push_back(gains[i]);
        }
    }
    
    // Configure each filter
    for (size_t i = 0; i < numFilters; ++i)
    {
        m_combFilters[i]->setDelay(delays[i]);
        m_combFilters[i]->setFeedback(feedbacks[i]);
        m_outputGains[i] = gains[i];
    }
}

void ParallelCombFilterBank::setDelaysInMs(const std::vector<float>& delaysInMs)
{
    std::vector<float> delaysInSamples;
    delaysInSamples.reserve(delaysInMs.size());
    
    for (float delayMs : delaysInMs)
    {
        float delaySamples = delayMs * static_cast<float>(m_sampleRate) / 1000.0f;
        delaysInSamples.push_back(delaySamples);
    }
    
    // Use default feedback and gain values
    std::vector<float> defaultFeedbacks(delaysInMs.size(), 0.7f);
    std::vector<float> defaultGains(delaysInMs.size(), 1.0f / static_cast<float>(delaysInMs.size()));
    
    setConfiguration(delaysInSamples, defaultFeedbacks, defaultGains);
}

void ParallelCombFilterBank::setGlobalFeedback(float feedback)
{
    for (auto& filter : m_combFilters)
    {
        filter->setFeedback(feedback);
    }
}

void ParallelCombFilterBank::setGlobalDamping(float damping)
{
    for (auto& filter : m_combFilters)
    {
        filter->setDamping(damping);
    }
}

float ParallelCombFilterBank::processSample(float inputSample)
{
    float output = 0.0f;
    
    // Process through each comb filter and sum the outputs
    for (size_t i = 0; i < m_combFilters.size(); ++i)
    {
        float filterOutput = m_combFilters[i]->processSample(inputSample);
        output += filterOutput * m_outputGains[i];
    }
    
    return output;
}

std::vector<float> ParallelCombFilterBank::generateCombDelays(size_t numFilters, float baseDelay) const
{
    std::vector<float> delays;
    delays.reserve(numFilters);
    
    // Generate delays that avoid harmonic relationships
    // Use prime number relationships for better decorrelation
    std::vector<float> primeFactors = { 1.0f, 1.1f, 1.3f, 1.7f, 1.9f, 2.3f, 2.9f, 3.1f };
    
    for (size_t i = 0; i < numFilters && i < primeFactors.size(); ++i)
    {
        delays.push_back(baseDelay * primeFactors[i]);
    }
    
    return delays;
}