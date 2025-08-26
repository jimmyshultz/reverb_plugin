#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>

/**
 * @brief A high-quality delay line with interpolation support
 * 
 * This delay line supports fractional delays using linear interpolation
 * for smooth modulation effects and precise delay times.
 */
class DelayLine
{
public:
    DelayLine();
    ~DelayLine() = default;
    
    /**
     * @brief Initialize the delay line with a specific capacity
     * @param maxDelayInSamples Maximum delay capacity in samples
     */
    void setMaxDelayInSamples(int maxDelayInSamples);
    
    /**
     * @brief Prepare for processing with specific sample rate
     * @param sampleRate The sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset the delay line, clearing all stored samples
     */
    void reset();
    
    /**
     * @brief Set the current delay time in samples (supports fractional delays)
     * @param delayInSamples Delay time in samples (can be fractional)
     */
    void setDelay(float delayInSamples);
    
    /**
     * @brief Set the delay time in milliseconds
     * @param delayInMs Delay time in milliseconds
     */
    void setDelayInMs(float delayInMs);
    
    /**
     * @brief Process a single sample through the delay line
     * @param inputSample The input sample to process
     * @return The delayed output sample
     */
    float processSample(float inputSample);
    
    /**
     * @brief Get the current delay time in samples
     * @return Current delay time in samples
     */
    float getDelay() const { return m_delayInSamples; }
    
    /**
     * @brief Get the maximum delay capacity
     * @return Maximum delay in samples
     */
    int getMaxDelay() const { return m_maxDelayInSamples; }
    
private:
    std::vector<float> m_buffer;
    int m_writeIndex;
    int m_maxDelayInSamples;
    float m_delayInSamples;
    double m_sampleRate;
    
    /**
     * @brief Perform linear interpolation between two samples
     * @param sample1 First sample
     * @param sample2 Second sample
     * @param fraction Fractional position between samples (0.0 - 1.0)
     * @return Interpolated sample value
     */
    float linearInterpolation(float sample1, float sample2, float fraction) const;
    
    /**
     * @brief Get the read index for the current delay time
     * @param delayInSamples Delay time in samples
     * @return Read index (may be fractional)
     */
    float getReadIndex(float delayInSamples) const;
};

/**
 * @brief Multi-tap delay line for early reflections
 * 
 * Provides multiple delay taps from a single delay line buffer
 * for creating early reflection patterns.
 */
class MultiTapDelayLine
{
public:
    struct DelayTap
    {
        float delayInSamples;
        float gain;
        float panPosition; // -1.0 (left) to 1.0 (right)
    };
    
    MultiTapDelayLine();
    ~MultiTapDelayLine() = default;
    
    /**
     * @brief Initialize with maximum delay capacity
     * @param maxDelayInSamples Maximum delay in samples
     */
    void setMaxDelayInSamples(int maxDelayInSamples);
    
    /**
     * @brief Prepare for processing
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset the delay line
     */
    void reset();
    
    /**
     * @brief Set the delay taps configuration
     * @param taps Vector of DelayTap structures
     */
    void setTaps(const std::vector<DelayTap>& taps);
    
    /**
     * @brief Process stereo input through multi-tap delay
     * @param leftInput Left channel input
     * @param rightInput Right channel input
     * @param leftOutput Reference to left channel output
     * @param rightOutput Reference to right channel output
     */
    void processStereo(float leftInput, float rightInput, 
                      float& leftOutput, float& rightOutput);
    
private:
    DelayLine m_delayLine;
    std::vector<DelayTap> m_taps;
    double m_sampleRate;
    
    /**
     * @brief Apply panning to a mono signal
     * @param input Mono input signal
     * @param panPosition Pan position (-1.0 to 1.0)
     * @param leftOutput Reference to left output
     * @param rightOutput Reference to right output
     */
    void applyPanning(float input, float panPosition, 
                     float& leftOutput, float& rightOutput) const;
};