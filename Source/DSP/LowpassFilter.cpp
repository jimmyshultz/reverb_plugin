#include "LowpassFilter.h"
#include <cmath>
#include <algorithm>

// LowpassFilter implementation
LowpassFilter::LowpassFilter()
    : m_cutoffFreq(1000.0f)
    , m_coefficient(0.5f)
    , m_previousOutput(0.0f)
    , m_sampleRate(44100.0)
{
}

void LowpassFilter::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    updateCoefficient();
}

void LowpassFilter::reset()
{
    m_previousOutput = 0.0f;
}

void LowpassFilter::setCutoffFrequency(float cutoffFreq)
{
    m_cutoffFreq = juce::jlimit(10.0f, static_cast<float>(m_sampleRate * 0.45), cutoffFreq);
    updateCoefficient();
}

void LowpassFilter::setCoefficient(float coefficient)
{
    m_coefficient = juce::jlimit(0.0f, 1.0f, coefficient);
}

float LowpassFilter::processSample(float inputSample)
{
    // One-pole lowpass filter: y[n] = (1-c) * x[n] + c * y[n-1]
    float output = (1.0f - m_coefficient) * inputSample + m_coefficient * m_previousOutput;
    m_previousOutput = output;
    return output;
}

void LowpassFilter::updateCoefficient()
{
    // Calculate coefficient from cutoff frequency
    float rc = 1.0f / (2.0f * juce::MathConstants<float>::pi * m_cutoffFreq);
    float dt = 1.0f / static_cast<float>(m_sampleRate);
    m_coefficient = rc / (rc + dt);
}

// TwoPoleFilter implementation
TwoPoleFilter::TwoPoleFilter()
    : m_cutoffFreq(1000.0f)
    , m_resonance(0.707f) // Butterworth response
    , m_a0(1.0f), m_a1(0.0f), m_a2(0.0f)
    , m_b1(0.0f), m_b2(0.0f)
    , m_x1(0.0f), m_x2(0.0f)
    , m_y1(0.0f), m_y2(0.0f)
    , m_sampleRate(44100.0)
{
}

void TwoPoleFilter::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    updateCoefficients();
}

void TwoPoleFilter::reset()
{
    m_x1 = m_x2 = 0.0f;
    m_y1 = m_y2 = 0.0f;
}

void TwoPoleFilter::setParameters(float cutoffFreq, float resonance)
{
    m_cutoffFreq = juce::jlimit(10.0f, static_cast<float>(m_sampleRate * 0.45), cutoffFreq);
    m_resonance = juce::jlimit(0.1f, 10.0f, resonance);
    updateCoefficients();
}

float TwoPoleFilter::processSample(float inputSample)
{
    // Biquad filter implementation
    float output = m_a0 * inputSample + m_a1 * m_x1 + m_a2 * m_x2 - m_b1 * m_y1 - m_b2 * m_y2;
    
    // Update delay lines
    m_x2 = m_x1;
    m_x1 = inputSample;
    m_y2 = m_y1;
    m_y1 = output;
    
    return output;
}

void TwoPoleFilter::updateCoefficients()
{
    // Calculate biquad coefficients for lowpass filter
    float omega = 2.0f * juce::MathConstants<float>::pi * m_cutoffFreq / static_cast<float>(m_sampleRate);
    float sin_omega = std::sin(omega);
    float cos_omega = std::cos(omega);
    float alpha = sin_omega / (2.0f * m_resonance);
    
    float b0 = 1.0f + alpha;
    float b1 = -2.0f * cos_omega;
    float b2 = 1.0f - alpha;
    
    float a0 = (1.0f - cos_omega) * 0.5f;
    float a1 = 1.0f - cos_omega;
    float a2 = (1.0f - cos_omega) * 0.5f;
    
    // Normalize coefficients
    m_a0 = a0 / b0;
    m_a1 = a1 / b0;
    m_a2 = a2 / b0;
    m_b1 = b1 / b0;
    m_b2 = b2 / b0;
}

// ShelfFilter implementation
ShelfFilter::ShelfFilter()
    : m_type(Type::HighShelf)
    , m_frequency(1000.0f)
    , m_gainDb(0.0f)
    , m_a0(1.0f), m_a1(0.0f), m_a2(0.0f)
    , m_b1(0.0f), m_b2(0.0f)
    , m_x1(0.0f), m_x2(0.0f)
    , m_y1(0.0f), m_y2(0.0f)
    , m_sampleRate(44100.0)
{
}

void ShelfFilter::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    updateCoefficients();
}

void ShelfFilter::reset()
{
    m_x1 = m_x2 = 0.0f;
    m_y1 = m_y2 = 0.0f;
}

void ShelfFilter::setParameters(Type type, float frequency, float gainDb)
{
    m_type = type;
    m_frequency = juce::jlimit(10.0f, static_cast<float>(m_sampleRate * 0.45), frequency);
    m_gainDb = juce::jlimit(-24.0f, 24.0f, gainDb);
    updateCoefficients();
}

float ShelfFilter::processSample(float inputSample)
{
    // Biquad filter implementation
    float output = m_a0 * inputSample + m_a1 * m_x1 + m_a2 * m_x2 - m_b1 * m_y1 - m_b2 * m_y2;
    
    // Update delay lines
    m_x2 = m_x1;
    m_x1 = inputSample;
    m_y2 = m_y1;
    m_y1 = output;
    
    return output;
}

void ShelfFilter::updateCoefficients()
{
    float A = std::pow(10.0f, m_gainDb / 40.0f); // Square root of gain
    float omega = 2.0f * juce::MathConstants<float>::pi * m_frequency / static_cast<float>(m_sampleRate);
    float sin_omega = std::sin(omega);
    float cos_omega = std::cos(omega);
    float S = 1.0f; // Shelf slope parameter
    float beta = std::sqrt(A) / S;
    
    if (m_type == Type::HighShelf)
    {
        // High shelf coefficients
        float b0 = (A + 1.0f) - (A - 1.0f) * cos_omega + beta * sin_omega;
        float b1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_omega);
        float b2 = (A + 1.0f) - (A - 1.0f) * cos_omega - beta * sin_omega;
        
        float a0 = A * ((A + 1.0f) + (A - 1.0f) * cos_omega + beta * sin_omega);
        float a1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_omega);
        float a2 = A * ((A + 1.0f) + (A - 1.0f) * cos_omega - beta * sin_omega);
        
        // Normalize
        m_a0 = a0 / b0;
        m_a1 = a1 / b0;
        m_a2 = a2 / b0;
        m_b1 = b1 / b0;
        m_b2 = b2 / b0;
    }
    else
    {
        // Low shelf coefficients
        float b0 = (A + 1.0f) + (A - 1.0f) * cos_omega + beta * sin_omega;
        float b1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_omega);
        float b2 = (A + 1.0f) + (A - 1.0f) * cos_omega - beta * sin_omega;
        
        float a0 = A * ((A + 1.0f) - (A - 1.0f) * cos_omega + beta * sin_omega);
        float a1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_omega);
        float a2 = A * ((A + 1.0f) - (A - 1.0f) * cos_omega - beta * sin_omega);
        
        // Normalize
        m_a0 = a0 / b0;
        m_a1 = a1 / b0;
        m_a2 = a2 / b0;
        m_b1 = b1 / b0;
        m_b2 = b2 / b0;
    }
}

// MultibandDampingFilter implementation
MultibandDampingFilter::MultibandDampingFilter()
    : m_lowMidCrossover(DEFAULT_LOW_MID_CROSSOVER)
    , m_midHighCrossover(DEFAULT_MID_HIGH_CROSSOVER)
    , m_sampleRate(44100.0)
{
}

void MultibandDampingFilter::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    
    m_lowBandFilter.prepare(sampleRate);
    m_midBandLowpass.prepare(sampleRate);
    m_midBandHighpass.prepare(sampleRate);
    m_highBandFilter.prepare(sampleRate);
    
    m_lowDampingFilter.prepare(sampleRate);
    m_midDampingFilter.prepare(sampleRate);
    m_highDampingFilter.prepare(sampleRate);
    
    updateBandFilters();
}

void MultibandDampingFilter::reset()
{
    m_lowBandFilter.reset();
    m_midBandLowpass.reset();
    m_midBandHighpass.reset();
    m_highBandFilter.reset();
    
    m_lowDampingFilter.reset();
    m_midDampingFilter.reset();
    m_highDampingFilter.reset();
}

void MultibandDampingFilter::setDamping(float lowDamping, float midDamping, float highDamping)
{
    // Convert damping to cutoff frequencies
    float lowCutoff = 20000.0f * (1.0f - lowDamping);
    float midCutoff = 20000.0f * (1.0f - midDamping);
    float highCutoff = 20000.0f * (1.0f - highDamping);
    
    m_lowDampingFilter.setCutoffFrequency(lowCutoff);
    m_midDampingFilter.setCutoffFrequency(midCutoff);
    m_highDampingFilter.setCutoffFrequency(highCutoff);
}

void MultibandDampingFilter::setCrossoverFrequencies(float lowMidCrossover, float midHighCrossover)
{
    m_lowMidCrossover = lowMidCrossover;
    m_midHighCrossover = midHighCrossover;
    updateBandFilters();
}

float MultibandDampingFilter::processSample(float inputSample)
{
    // This is a simplified approach - a real multiband filter would require
    // more complex band separation and reconstruction
    
    // For now, apply a single damping filter based on frequency content
    float output = m_highDampingFilter.processSample(inputSample);
    
    return output;
}

void MultibandDampingFilter::updateBandFilters()
{
    m_lowBandFilter.setCutoffFrequency(m_lowMidCrossover);
    m_midBandLowpass.setParameters(m_midHighCrossover, 0.707f);
    m_midBandHighpass.setParameters(m_lowMidCrossover, 0.707f);
    m_highBandFilter.setCutoffFrequency(m_midHighCrossover);
}