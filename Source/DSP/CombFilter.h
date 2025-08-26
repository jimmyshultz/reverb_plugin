#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"

// Forward declaration
class LowpassFilter;

/**
 * @brief Comb filter for reverb feedback processing
 * 
 * A comb filter creates a series of regularly spaced peaks and nulls
 * in the frequency response, resembling a comb. It's fundamental to
 * reverb algorithms for creating the dense, sustained tail.
 */
class CombFilter
{
public:
    CombFilter();
    ~CombFilter() = default;
    
    /**
     * @brief Prepare the comb filter for processing
     * @param sampleRate Sample rate for processing
     * @param maxDelayInSamples Maximum delay capacity in samples
     */
    void prepare(double sampleRate, int maxDelayInSamples);
    
    /**
     * @brief Reset the filter state
     */
    void reset();
    
    /**
     * @brief Set the delay time in samples
     * @param delayInSamples Delay time in samples
     */
    void setDelay(float delayInSamples);
    
    /**
     * @brief Set the delay time in milliseconds
     * @param delayInMs Delay time in milliseconds
     */
    void setDelayInMs(float delayInMs);
    
    /**
     * @brief Set the feedback gain
     * @param gain Feedback gain (0.0 to 0.99)
     */
    void setFeedback(float gain);
    
    /**
     * @brief Set the feedforward gain
     * @param gain Feedforward gain (0.0 to 1.0)
     */
    void setFeedforward(float gain);
    
    /**
     * @brief Process a single sample through the comb filter
     * @param inputSample Input sample to process
     * @return Processed output sample
     */
    float processSample(float inputSample);
    
    /**
     * @brief Get the current delay time in samples
     * @return Current delay time in samples
     */
    float getDelay() const { return m_delayLine.getDelay(); }
    
    /**
     * @brief Get the current feedback gain
     * @return Current feedback gain
     */
    float getFeedback() const { return m_feedbackGain; }
    
    /**
     * @brief Get the current feedforward gain
     * @return Current feedforward gain
     */
    float getFeedforward() const { return m_feedforwardGain; }
    
private:
    DelayLine m_delayLine;
    float m_feedbackGain;
    float m_feedforwardGain;
    double m_sampleRate;
    float m_lastOutput; // For stability
};

/**
 * @brief Damped comb filter with integrated lowpass filtering
 * 
 * This comb filter includes a lowpass filter in the feedback path
 * to simulate frequency-dependent absorption in reverberant spaces.
 */
class DampedCombFilter
{
public:
    DampedCombFilter();
    ~DampedCombFilter() = default;
    
    /**
     * @brief Prepare the damped comb filter
     * @param sampleRate Sample rate for processing
     * @param maxDelayInSamples Maximum delay capacity
     */
    void prepare(double sampleRate, int maxDelayInSamples);
    
    /**
     * @brief Reset the filter state
     */
    void reset();
    
    /**
     * @brief Set the delay time in samples
     * @param delayInSamples Delay time in samples
     */
    void setDelay(float delayInSamples);
    
    /**
     * @brief Set the delay time in milliseconds
     * @param delayInMs Delay time in milliseconds
     */
    void setDelayInMs(float delayInMs);
    
    /**
     * @brief Set the feedback gain
     * @param gain Feedback gain
     */
    void setFeedback(float gain);
    
    /**
     * @brief Set the damping amount (lowpass filter cutoff)
     * @param damping Damping amount (0.0 = no damping, 1.0 = heavy damping)
     */
    void setDamping(float damping);
    
    /**
     * @brief Process a single sample
     * @param inputSample Input sample
     * @return Processed output sample
     */
    float processSample(float inputSample);
    
private:
    CombFilter m_combFilter;
    LowpassFilter m_dampingFilter;
    double m_sampleRate;
    
    /**
     * @brief Convert damping parameter to cutoff frequency
     * @param damping Damping amount (0.0 to 1.0)
     * @return Cutoff frequency in Hz
     */
    float dampingToCutoff(float damping) const;
};

/**
 * @brief Parallel comb filter bank
 * 
 * Multiple comb filters running in parallel, as used in
 * Schroeder reverb and other classic reverb algorithms.
 */
class ParallelCombFilterBank
{
public:
    ParallelCombFilterBank();
    ~ParallelCombFilterBank() = default;
    
    /**
     * @brief Prepare the filter bank
     * @param sampleRate Sample rate
     * @param maxDelayInSamples Maximum delay for each comb filter
     */
    void prepare(double sampleRate, int maxDelayInSamples);
    
    /**
     * @brief Reset all comb filters
     */
    void reset();
    
    /**
     * @brief Configure the comb filter bank
     * @param delays Vector of delay times in samples
     * @param feedbacks Vector of feedback gains
     * @param gains Vector of output gains for mixing
     */
    void setConfiguration(const std::vector<float>& delays,
                         const std::vector<float>& feedbacks,
                         const std::vector<float>& gains);
    
    /**
     * @brief Set delays in milliseconds
     * @param delaysInMs Vector of delay times in milliseconds
     */
    void setDelaysInMs(const std::vector<float>& delaysInMs);
    
    /**
     * @brief Set global feedback for all filters
     * @param feedback Global feedback amount
     */
    void setGlobalFeedback(float feedback);
    
    /**
     * @brief Set global damping for all filters (if using damped combs)
     * @param damping Global damping amount
     */
    void setGlobalDamping(float damping);
    
    /**
     * @brief Process a sample through the entire bank
     * @param inputSample Input sample
     * @return Mixed output from all comb filters
     */
    float processSample(float inputSample);
    
    /**
     * @brief Get the number of comb filters in the bank
     * @return Number of comb filters
     */
    size_t getNumFilters() const { return m_combFilters.size(); }
    
private:
    std::vector<std::unique_ptr<DampedCombFilter>> m_combFilters;
    std::vector<float> m_outputGains;
    double m_sampleRate;
    
    static constexpr size_t MAX_COMB_FILTERS = 8;
    
    /**
     * @brief Generate appropriate delay times for comb filters
     * @param numFilters Number of filters to generate delays for
     * @param baseDelay Base delay time in samples
     * @return Vector of delay times designed to avoid resonances
     */
    std::vector<float> generateCombDelays(size_t numFilters, float baseDelay) const;
};