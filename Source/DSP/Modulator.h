#pragma once

#include <JuceHeader.h>
#include <array>

/**
 * @brief Low Frequency Oscillator (LFO) for modulation effects
 * 
 * Provides various waveforms for modulating parameters in the reverb
 * to create movement and reduce metallic artifacts.
 */
class LFO
{
public:
    enum class Waveform
    {
        Sine,
        Triangle,
        Sawtooth,
        Square,
        Noise
    };
    
    LFO();
    ~LFO() = default;
    
    /**
     * @brief Prepare the LFO for processing
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset the LFO phase
     */
    void reset();
    
    /**
     * @brief Set the oscillator frequency
     * @param frequency Frequency in Hz
     */
    void setFrequency(float frequency);
    
    /**
     * @brief Set the waveform type
     * @param waveform Waveform type
     */
    void setWaveform(Waveform waveform);
    
    /**
     * @brief Set the initial phase offset
     * @param phase Phase offset in radians (0 to 2π)
     */
    void setPhase(float phase);
    
    /**
     * @brief Generate the next sample
     * @return LFO output value (-1.0 to 1.0)
     */
    float getNextSample();
    
    /**
     * @brief Get the current phase
     * @return Current phase in radians
     */
    float getCurrentPhase() const { return m_phase; }
    
private:
    float m_frequency;
    float m_phase;
    float m_phaseIncrement;
    Waveform m_waveform;
    double m_sampleRate;
    
    // For noise generation
    juce::Random m_random;
    float m_noiseValue;
    int m_noiseSampleCounter;
    static constexpr int NOISE_UPDATE_RATE = 64; // Update noise every N samples
    
    /**
     * @brief Update the phase increment based on frequency and sample rate
     */
    void updatePhaseIncrement();
    
    /**
     * @brief Generate waveform sample for current phase
     * @param phase Current phase in radians
     * @return Waveform sample value
     */
    float generateWaveform(float phase) const;
};

/**
 * @brief Multi-LFO for complex modulation patterns
 * 
 * Multiple LFOs that can be combined for richer modulation effects.
 */
class MultiLFO
{
public:
    MultiLFO();
    ~MultiLFO() = default;
    
    /**
     * @brief Prepare all LFOs for processing
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset all LFOs
     */
    void reset();
    
    /**
     * @brief Configure the LFOs
     * @param frequencies Vector of frequencies for each LFO
     * @param waveforms Vector of waveforms for each LFO
     * @param phases Vector of phase offsets for each LFO
     * @param weights Vector of mixing weights for each LFO
     */
    void configure(const std::vector<float>& frequencies,
                  const std::vector<LFO::Waveform>& waveforms,
                  const std::vector<float>& phases,
                  const std::vector<float>& weights);
    
    /**
     * @brief Get the combined output of all LFOs
     * @return Mixed LFO output
     */
    float getNextSample();
    
    /**
     * @brief Get the output of a specific LFO
     * @param index LFO index
     * @return Individual LFO output
     */
    float getLFOOutput(size_t index);
    
    /**
     * @brief Get the number of active LFOs
     * @return Number of LFOs
     */
    size_t getNumLFOs() const { return m_lfos.size(); }
    
private:
    std::vector<std::unique_ptr<LFO>> m_lfos;
    std::vector<float> m_weights;
    double m_sampleRate;
    
    static constexpr size_t MAX_LFOS = 4;
};

/**
 * @brief Modulation matrix for flexible routing
 * 
 * Allows routing of modulation sources to multiple destinations
 * with configurable amounts.
 */
class ModulationMatrix
{
public:
    enum class Source
    {
        LFO1,
        LFO2,
        LFO3,
        LFO4,
        Envelope,
        Random
    };
    
    enum class Destination
    {
        DelayTime,
        Feedback,
        Diffusion,
        Damping,
        PanPosition,
        Pitch
    };
    
    struct ModulationRoute
    {
        Source source;
        Destination destination;
        float amount; // -1.0 to 1.0
        bool enabled;
    };
    
    ModulationMatrix();
    ~ModulationMatrix() = default;
    
    /**
     * @brief Prepare the modulation matrix
     * @param sampleRate Sample rate for processing
     */
    void prepare(double sampleRate);
    
    /**
     * @brief Reset all modulation sources
     */
    void reset();
    
    /**
     * @brief Add or update a modulation route
     * @param route Modulation route configuration
     */
    void setRoute(const ModulationRoute& route);
    
    /**
     * @brief Remove a modulation route
     * @param source Source to remove
     * @param destination Destination to remove
     */
    void removeRoute(Source source, Destination destination);
    
    /**
     * @brief Process one sample and update all modulation values
     */
    void processSample();
    
    /**
     * @brief Get the current modulation value for a destination
     * @param destination Target destination
     * @return Modulation value (-1.0 to 1.0)
     */
    float getModulationValue(Destination destination) const;
    
    /**
     * @brief Get the MultiLFO for configuration
     * @return Reference to the internal MultiLFO
     */
    MultiLFO& getMultiLFO() { return m_multiLFO; }
    
private:
    MultiLFO m_multiLFO;
    std::vector<ModulationRoute> m_routes;
    std::array<float, 6> m_destinationValues; // One for each destination
    
    juce::Random m_random;
    float m_envelopeValue;
    
    /**
     * @brief Update all destination values based on current routes
     */
    void updateDestinations();
    
    /**
     * @brief Get the current value for a modulation source
     * @param source Modulation source
     * @return Source value (-1.0 to 1.0)
     */
    float getSourceValue(Source source) const;
};

/**
 * @brief Pitch modulation for chorus-like effects
 * 
 * Applies subtle pitch modulation to create movement and width
 * in the reverb tail.
 */
class PitchModulator
{
public:
    PitchModulator();
    ~PitchModulator() = default;
    
    /**
     * @brief Prepare the pitch modulator
     * @param sampleRate Sample rate for processing
     * @param maxDelayInSamples Maximum delay for pitch shifting
     */
    void prepare(double sampleRate, int maxDelayInSamples);
    
    /**
     * @brief Reset the modulator state
     */
    void reset();
    
    /**
     * @brief Set the modulation parameters
     * @param rate Modulation rate in Hz
     * @param depth Pitch modulation depth in cents (±100 cents = ±1 semitone)
     */
    void setModulation(float rate, float depth);
    
    /**
     * @brief Process a sample with pitch modulation
     * @param inputSample Input sample
     * @return Pitch-modulated output sample
     */
    float processSample(float inputSample);
    
private:
    LFO m_lfo;
    float m_rate;
    float m_depth;
    double m_sampleRate;
    
    // Simple delay line for pitch shifting
    std::vector<float> m_delayBuffer;
    int m_writeIndex;
    int m_maxDelayInSamples;
    
    /**
     * @brief Convert cents to delay time ratio
     * @param cents Pitch shift in cents
     * @return Delay time multiplier
     */
    float centsToDelayRatio(float cents) const;
    
    /**
     * @brief Interpolated read from delay buffer
     * @param delayInSamples Fractional delay time
     * @return Interpolated sample
     */
    float readDelayBuffer(float delayInSamples) const;
};