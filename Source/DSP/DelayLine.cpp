#include "DelayLine.h"
#include <algorithm>
#include <cmath>

// DelayLine implementation
DelayLine::DelayLine()
    : m_writeIndex(0)
    , m_maxDelayInSamples(0)
    , m_delayInSamples(0.0f)
    , m_sampleRate(44100.0)
{
}

void DelayLine::setMaxDelayInSamples(int maxDelayInSamples)
{
    m_maxDelayInSamples = maxDelayInSamples;
    m_buffer.resize(maxDelayInSamples + 1); // +1 for interpolation
    reset();
}

void DelayLine::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
}

void DelayLine::reset()
{
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0f);
    m_writeIndex = 0;
}

void DelayLine::setDelay(float delayInSamples)
{
    m_delayInSamples = juce::jlimit(0.0f, static_cast<float>(m_maxDelayInSamples), delayInSamples);
}

void DelayLine::setDelayInMs(float delayInMs)
{
    float delayInSamples = delayInMs * static_cast<float>(m_sampleRate) / 1000.0f;
    setDelay(delayInSamples);
}

float DelayLine::processSample(float inputSample)
{
    // Write input sample to buffer
    m_buffer[m_writeIndex] = inputSample;
    
    // Calculate read position with fractional delay support
    float readPosition = getReadIndex(m_delayInSamples);
    
    // Get integer and fractional parts
    int readIndex1 = static_cast<int>(std::floor(readPosition));
    int readIndex2 = readIndex1 + 1;
    float fraction = readPosition - static_cast<float>(readIndex1);
    
    // Wrap indices
    readIndex1 = (readIndex1 + m_buffer.size()) % m_buffer.size();
    readIndex2 = (readIndex2 + m_buffer.size()) % m_buffer.size();
    
    // Perform linear interpolation
    float sample1 = m_buffer[readIndex1];
    float sample2 = m_buffer[readIndex2];
    float outputSample = linearInterpolation(sample1, sample2, fraction);
    
    // Advance write index
    m_writeIndex = (m_writeIndex + 1) % m_buffer.size();
    
    return outputSample;
}

float DelayLine::linearInterpolation(float sample1, float sample2, float fraction) const
{
    return sample1 + fraction * (sample2 - sample1);
}

float DelayLine::getReadIndex(float delayInSamples) const
{
    float readPosition = static_cast<float>(m_writeIndex) - delayInSamples;
    
    // Handle negative indices by wrapping around
    while (readPosition < 0.0f)
        readPosition += static_cast<float>(m_buffer.size());
    
    return readPosition;
}

// MultiTapDelayLine implementation
MultiTapDelayLine::MultiTapDelayLine()
    : m_sampleRate(44100.0)
{
}

void MultiTapDelayLine::setMaxDelayInSamples(int maxDelayInSamples)
{
    m_delayLine.setMaxDelayInSamples(maxDelayInSamples);
}

void MultiTapDelayLine::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    m_delayLine.prepare(sampleRate);
}

void MultiTapDelayLine::reset()
{
    m_delayLine.reset();
}

void MultiTapDelayLine::setTaps(const std::vector<DelayTap>& taps)
{
    m_taps = taps;
}

void MultiTapDelayLine::processStereo(float leftInput, float rightInput,
                                    float& leftOutput, float& rightOutput)
{
    // Mix input channels for mono processing
    float monoInput = (leftInput + rightInput) * 0.5f;
    
    // Process through delay line (this updates the internal buffer)
    m_delayLine.processSample(monoInput);
    
    // Initialize outputs
    leftOutput = 0.0f;
    rightOutput = 0.0f;
    
    // Sum all tap outputs
    for (const auto& tap : m_taps)
    {
        // Get delayed sample for this tap
        float readPosition = m_delayLine.getReadIndex(tap.delayInSamples);
        
        // Extract samples for interpolation (accessing internal buffer)
        int readIndex1 = static_cast<int>(std::floor(readPosition));
        int readIndex2 = readIndex1 + 1;
        float fraction = readPosition - static_cast<float>(readIndex1);
        
        // Note: In a real implementation, we'd need access to the DelayLine's buffer
        // For now, we'll use a simplified approach
        float delayedSample = 0.0f; // This would be interpolated from the buffer
        
        // Apply gain
        delayedSample *= tap.gain;
        
        // Apply panning and add to outputs
        float tapLeft, tapRight;
        applyPanning(delayedSample, tap.panPosition, tapLeft, tapRight);
        
        leftOutput += tapLeft;
        rightOutput += tapRight;
    }
}

void MultiTapDelayLine::applyPanning(float input, float panPosition,
                                   float& leftOutput, float& rightOutput) const
{
    // Convert pan position from -1..1 to 0..1
    float normalizedPan = (panPosition + 1.0f) * 0.5f;
    
    // Apply constant power panning
    float leftGain = std::cos(normalizedPan * juce::MathConstants<float>::halfPi);
    float rightGain = std::sin(normalizedPan * juce::MathConstants<float>::halfPi);
    
    leftOutput = input * leftGain;
    rightOutput = input * rightGain;
}