#pragma once

#include <JuceHeader.h>
#include <array>

/**
 * @brief Simple one-pole lowpass filter for damping
 * 
 * A basic first-order lowpass filter commonly used in reverb
 * algorithms to simulate high-frequency absorption.
 */
class LowpassFilter
{
public:
    LowpassFilter();
    ~LowpassFilter() = default;
    
    /**
     * @brief Prepare the filter for processing
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset the filter state
     */
    void reset();
    
    /**
     * @brief Set the cutoff frequency
     * @param cutoffFreq Cutoff frequency in Hz
     */
    void setCutoffFrequency(float cutoffFreq);
    
    /**
     * @brief Set the filter coefficient directly
     * @param coefficient Filter coefficient (0.0 to 1.0)
     */
    void setCoefficient(float coefficient);
    
    /**
     * @brief Process a single sample
     * @param inputSample Input sample to process
     * @return Filtered output sample
     */
    float processSample(float inputSample);
    
    /**
     * @brief Get the current cutoff frequency
     * @return Current cutoff frequency in Hz
     */
    float getCutoffFrequency() const { return m_cutoffFreq; }
    
private:
    float m_cutoffFreq;
    float m_coefficient;
    float m_previousOutput;
    double m_sampleRate;
    
    /**
     * @brief Calculate filter coefficient from cutoff frequency
     */
    void updateCoefficient();
};

/**
 * @brief Two-pole lowpass filter with adjustable Q
 * 
 * A second-order lowpass filter providing steeper rolloff
 * and resonance control for more complex filtering effects.
 */
class TwoPoleFilter
{
public:
    TwoPoleFilter();
    ~TwoPoleFilter() = default;
    
    /**
     * @brief Prepare the filter for processing
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset the filter state
     */
    void reset();
    
    /**
     * @brief Set the filter parameters
     * @param cutoffFreq Cutoff frequency in Hz
     * @param resonance Resonance/Q factor (0.1 to 10.0)
     */
    void setParameters(float cutoffFreq, float resonance);
    
    /**
     * @brief Process a single sample
     * @param inputSample Input sample to process
     * @return Filtered output sample
     */
    float processSample(float inputSample);
    
    /**
     * @brief Get the current cutoff frequency
     * @return Current cutoff frequency in Hz
     */
    float getCutoffFrequency() const { return m_cutoffFreq; }
    
    /**
     * @brief Get the current resonance
     * @return Current resonance value
     */
    float getResonance() const { return m_resonance; }
    
private:
    float m_cutoffFreq;
    float m_resonance;
    
    // Biquad filter coefficients
    float m_a0, m_a1, m_a2;
    float m_b1, m_b2;
    
    // Filter state
    float m_x1, m_x2; // Input delay line
    float m_y1, m_y2; // Output delay line
    
    double m_sampleRate;
    
    /**
     * @brief Calculate biquad coefficients
     */
    void updateCoefficients();
};

/**
 * @brief Shelf filter for tone control
 * 
 * A shelving filter that can boost or cut frequencies
 * above or below a specified frequency.
 */
class ShelfFilter
{
public:
    enum class Type
    {
        LowShelf,
        HighShelf
    };
    
    ShelfFilter();
    ~ShelfFilter() = default;
    
    /**
     * @brief Prepare the filter for processing
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset the filter state
     */
    void reset();
    
    /**
     * @brief Set the filter parameters
     * @param type Filter type (low or high shelf)
     * @param frequency Shelf frequency in Hz
     * @param gainDb Gain in dB (positive = boost, negative = cut)
     */
    void setParameters(Type type, float frequency, float gainDb);
    
    /**
     * @brief Process a single sample
     * @param inputSample Input sample to process
     * @return Filtered output sample
     */
    float processSample(float inputSample);
    
private:
    Type m_type;
    float m_frequency;
    float m_gainDb;
    
    // Biquad coefficients
    float m_a0, m_a1, m_a2;
    float m_b1, m_b2;
    
    // Filter state
    float m_x1, m_x2;
    float m_y1, m_y2;
    
    double m_sampleRate;
    
    /**
     * @brief Calculate shelf filter coefficients
     */
    void updateCoefficients();
};

/**
 * @brief Multi-band damping filter
 * 
 * Provides frequency-dependent damping across multiple bands
 * for realistic reverb tail shaping.
 */
class MultibandDampingFilter
{
public:
    MultibandDampingFilter();
    ~MultibandDampingFilter() = default;
    
    /**
     * @brief Prepare the filter for processing
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset all filter states
     */
    void reset();
    
    /**
     * @brief Set the damping parameters for each band
     * @param lowDamping Low frequency damping (0.0 to 1.0)
     * @param midDamping Mid frequency damping (0.0 to 1.0)
     * @param highDamping High frequency damping (0.0 to 1.0)
     */
    void setDamping(float lowDamping, float midDamping, float highDamping);
    
    /**
     * @brief Set the crossover frequencies
     * @param lowMidCrossover Low-mid crossover frequency in Hz
     * @param midHighCrossover Mid-high crossover frequency in Hz
     */
    void setCrossoverFrequencies(float lowMidCrossover, float midHighCrossover);
    
    /**
     * @brief Process a single sample
     * @param inputSample Input sample to process
     * @return Processed output sample
     */
    float processSample(float inputSample);
    
private:
    // Band separation filters
    LowpassFilter m_lowBandFilter;
    TwoPoleFilter m_midBandLowpass;
    TwoPoleFilter m_midBandHighpass;
    LowpassFilter m_highBandFilter;
    
    // Damping filters for each band
    LowpassFilter m_lowDampingFilter;
    LowpassFilter m_midDampingFilter;
    LowpassFilter m_highDampingFilter;
    
    float m_lowMidCrossover;
    float m_midHighCrossover;
    
    double m_sampleRate;
    
    static constexpr float DEFAULT_LOW_MID_CROSSOVER = 500.0f;
    static constexpr float DEFAULT_MID_HIGH_CROSSOVER = 5000.0f;
    
    /**
     * @brief Update filter parameters based on crossover frequencies
     */
    void updateBandFilters();
};