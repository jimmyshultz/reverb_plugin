#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"

/**
 * @brief Allpass filter for reverb diffusion
 * 
 * An allpass filter maintains the amplitude response while changing
 * the phase response. It's commonly used in reverb algorithms to
 * create diffusion and break up flutter echoes.
 */
class AllpassFilter
{
public:
    AllpassFilter();
    ~AllpassFilter() = default;
    
    /**
     * @brief Prepare the allpass filter for processing
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
     * @brief Set the feedback/feedforward gain
     * @param gain Gain value (typically -0.7 to 0.7)
     */
    void setGain(float gain);
    
    /**
     * @brief Process a single sample through the allpass filter
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
     * @brief Get the current gain setting
     * @return Current gain value
     */
    float getGain() const { return m_gain; }
    
private:
    DelayLine m_delayLine;
    float m_gain;
    double m_sampleRate;
};

/**
 * @brief Modulated allpass filter for chorus-like effects
 * 
 * This allpass filter includes modulation of the delay time
 * to create movement and reduce metallic artifacts in reverb.
 */
class ModulatedAllpassFilter
{
public:
    ModulatedAllpassFilter();
    ~ModulatedAllpassFilter() = default;
    
    /**
     * @brief Prepare the modulated allpass filter
     * @param sampleRate Sample rate for processing
     * @param maxDelayInSamples Maximum delay capacity
     */
    void prepare(double sampleRate, int maxDelayInSamples);
    
    /**
     * @brief Reset the filter state
     */
    void reset();
    
    /**
     * @brief Set the base delay time in samples
     * @param delayInSamples Base delay time in samples
     */
    void setDelay(float delayInSamples);
    
    /**
     * @brief Set the modulation parameters
     * @param rate Modulation rate in Hz
     * @param depth Modulation depth as fraction of delay time (0.0 to 1.0)
     */
    void setModulation(float rate, float depth);
    
    /**
     * @brief Set the feedback/feedforward gain
     * @param gain Gain value
     */
    void setGain(float gain);
    
    /**
     * @brief Process a single sample
     * @param inputSample Input sample
     * @return Processed output sample
     */
    float processSample(float inputSample);
    
private:
    AllpassFilter m_allpassFilter;
    float m_baseDelayInSamples;
    float m_modulationRate;
    float m_modulationDepth;
    float m_phase;
    double m_sampleRate;
    
    /**
     * @brief Update the modulated delay time
     */
    void updateModulatedDelay();
};

/**
 * @brief Nested allpass filter network
 * 
 * Multiple allpass filters in series for increased diffusion.
 * Commonly used in high-quality reverb algorithms.
 */
class NestedAllpassNetwork
{
public:
    NestedAllpassNetwork();
    ~NestedAllpassNetwork() = default;
    
    /**
     * @brief Prepare the network for processing
     * @param sampleRate Sample rate
     * @param maxDelayInSamples Maximum delay for each stage
     */
    void prepare(double sampleRate, int maxDelayInSamples);
    
    /**
     * @brief Reset all filters in the network
     */
    void reset();
    
    /**
     * @brief Set the configuration for all allpass stages
     * @param delays Vector of delay times in samples for each stage
     * @param gains Vector of gain values for each stage
     */
    void setConfiguration(const std::vector<float>& delays, const std::vector<float>& gains);
    
    /**
     * @brief Set delays in milliseconds
     * @param delaysInMs Vector of delay times in milliseconds
     */
    void setDelaysInMs(const std::vector<float>& delaysInMs);
    
    /**
     * @brief Process a sample through the entire network
     * @param inputSample Input sample
     * @return Processed output sample
     */
    float processSample(float inputSample);
    
    /**
     * @brief Get the number of stages in the network
     * @return Number of allpass filter stages
     */
    size_t getNumStages() const { return m_allpassFilters.size(); }
    
private:
    std::vector<std::unique_ptr<AllpassFilter>> m_allpassFilters;
    double m_sampleRate;
    
    static constexpr size_t MAX_STAGES = 8;
};