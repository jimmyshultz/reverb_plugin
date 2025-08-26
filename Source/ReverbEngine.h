#pragma once

#include <JuceHeader.h>
#include "DSP/DelayLine.h"
#include "DSP/AllpassFilter.h"
#include "DSP/CombFilter.h"
#include "DSP/LowpassFilter.h"
#include "DSP/Modulator.h"
#include <memory>

/**
 * @brief Core reverb engine implementing Schroeder reverb topology
 * 
 * This class implements a high-quality reverb algorithm using the classic
 * Schroeder topology with modern enhancements including:
 * - Pre-delay buffer
 * - Early reflections processing
 * - Parallel comb filter bank for late reverb
 * - Series allpass filters for diffusion
 * - Modulation for movement and artifact reduction
 * - Frequency-dependent damping
 */
class ReverbEngine
{
public:
    ReverbEngine();
    ~ReverbEngine() = default;
    
    /**
     * @brief Prepare the reverb engine for processing
     * @param sampleRate Sample rate for processing
     * @param expectedBlockSize Expected audio buffer size
     * @param numChannels Number of audio channels (1 or 2)
     */
    void prepare(double sampleRate, int expectedBlockSize, int numChannels);
    
    /**
     * @brief Reset all internal states
     */
    void reset();
    
    /**
     * @brief Process an audio buffer
     * @param buffer Audio buffer to process (input/output)
     */
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    /**
     * @brief Process stereo samples
     * @param leftInput Left channel input
     * @param rightInput Right channel input
     * @param leftOutput Reference to left channel output
     * @param rightOutput Reference to right channel output
     */
    void processStereo(float leftInput, float rightInput, 
                      float& leftOutput, float& rightOutput);
    
    // Parameter setters
    void setRoomSize(float roomSize);
    void setDecayTime(float decayTime);
    void setPreDelay(float preDelayMs);
    void setDamping(float damping);
    void setWetDryMix(float wetDryMix);
    void setEarlyLateBalance(float balance);
    void setDiffusion(float diffusion);
    void setModulationRate(float rate);
    void setModulationDepth(float depth);
    
    // Parameter getters
    float getRoomSize() const { return m_roomSize; }
    float getDecayTime() const { return m_decayTime; }
    float getPreDelay() const { return m_preDelayMs; }
    float getDamping() const { return m_damping; }
    float getWetDryMix() const { return m_wetDryMix; }
    float getEarlyLateBalance() const { return m_earlyLateBalance; }
    float getDiffusion() const { return m_diffusion; }
    float getModulationRate() const { return m_modulationRate; }
    float getModulationDepth() const { return m_modulationDepth; }
    
private:
    // Audio processing components
    std::unique_ptr<DelayLine> m_preDelayLeft;
    std::unique_ptr<DelayLine> m_preDelayRight;
    std::unique_ptr<MultiTapDelayLine> m_earlyReflections;
    std::unique_ptr<ParallelCombFilterBank> m_combFilterBank;
    std::unique_ptr<NestedAllpassNetwork> m_allpassNetwork;
    std::unique_ptr<ModulationMatrix> m_modulationMatrix;
    
    // Parameters
    float m_roomSize;
    float m_decayTime;
    float m_preDelayMs;
    float m_damping;
    float m_wetDryMix;
    float m_earlyLateBalance;
    float m_diffusion;
    float m_modulationRate;
    float m_modulationDepth;
    
    // Internal state
    double m_sampleRate;
    int m_numChannels;
    bool m_isPrepared;
    
    // Constants for reverb design
    static constexpr float MAX_ROOM_SIZE = 10.0f;
    static constexpr float MAX_DECAY_TIME = 20.0f;
    static constexpr float MAX_PRE_DELAY_MS = 500.0f;
    static constexpr int BASE_COMB_DELAY_MS = 30;
    static constexpr int BASE_ALLPASS_DELAY_MS = 5;
    
    /**
     * @brief Initialize the reverb algorithm components
     */
    void initializeComponents();
    
    /**
     * @brief Configure early reflections pattern
     */
    void configureEarlyReflections();
    
    /**
     * @brief Configure comb filter delays and parameters
     */
    void configureCombFilters();
    
    /**
     * @brief Configure allpass filter network
     */
    void configureAllpassFilters();
    
    /**
     * @brief Configure modulation routing
     */
    void configureModulation();
    
    /**
     * @brief Update all parameters based on current settings
     */
    void updateParameters();
    
    /**
     * @brief Calculate room size dependent delays
     * @param baseDelayMs Base delay time in milliseconds
     * @return Scaled delay time based on room size
     */
    float calculateRoomScaledDelay(float baseDelayMs) const;
    
    /**
     * @brief Calculate decay time dependent feedback
     * @param baseDecay Base decay time
     * @return Feedback gain for the specified decay time
     */
    float calculateDecayFeedback(float baseDecay) const;
    
    /**
     * @brief Process early reflections
     * @param leftInput Left channel input
     * @param rightInput Right channel input
     * @param leftOutput Reference to left channel output
     * @param rightOutput Reference to right channel output
     */
    void processEarlyReflections(float leftInput, float rightInput,
                               float& leftOutput, float& rightOutput);
    
    /**
     * @brief Process late reverb (comb filters + allpass)
     * @param input Mono input signal
     * @return Processed reverb tail
     */
    float processLateReverb(float input);
    
    /**
     * @brief Apply wet/dry mixing
     * @param drySignal Original dry signal
     * @param wetSignal Processed wet signal
     * @return Mixed output signal
     */
    float applyWetDryMix(float drySignal, float wetSignal) const;
    
    /**
     * @brief Apply stereo width processing
     * @param leftSignal Left channel signal
     * @param rightSignal Right channel signal
     * @param leftOutput Reference to left output
     * @param rightOutput Reference to right output
     * @param width Stereo width (0.0 = mono, 1.0 = full stereo)
     */
    void applyStereoWidth(float leftSignal, float rightSignal,
                         float& leftOutput, float& rightOutput,
                         float width) const;
};

/**
 * @brief Preset configuration for common reverb types
 * 
 * Provides factory presets that configure the reverb engine
 * for specific acoustic spaces and reverb effects.
 */
class ReverbPreset
{
public:
    struct PresetData
    {
        juce::String name;
        float roomSize;
        float decayTime;
        float preDelay;
        float damping;
        float wetDryMix;
        float earlyLateBalance;
        float diffusion;
        float modulationRate;
        float modulationDepth;
    };
    
    /**
     * @brief Get all available factory presets
     * @return Vector of preset configurations
     */
    static std::vector<PresetData> getFactoryPresets();
    
    /**
     * @brief Apply a preset to a reverb engine
     * @param engine Reverb engine to configure
     * @param preset Preset data to apply
     */
    static void applyPreset(ReverbEngine& engine, const PresetData& preset);
    
    /**
     * @brief Create a preset from current engine settings
     * @param engine Reverb engine to read settings from
     * @param name Name for the new preset
     * @return Preset data structure
     */
    static PresetData createPresetFromEngine(const ReverbEngine& engine, const juce::String& name);
    
private:
    /**
     * @brief Initialize factory preset definitions
     */
    static void initializeFactoryPresets();
    
    static std::vector<PresetData> s_factoryPresets;
    static bool s_presetsInitialized;
};