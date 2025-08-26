#pragma once

#include <JuceHeader.h>
#include "ReverbEngine.h"
#include "Parameters.h"
#include "PresetManager.h"

/**
 * @brief Main audio processor for the Reverb Plugin
 * 
 * This class handles all audio processing, parameter management,
 * and plugin interface implementation using the JUCE framework.
 */
class ReverbPluginAudioProcessor : public juce::AudioProcessor
{
public:
    ReverbPluginAudioProcessor();
    ~ReverbPluginAudioProcessor() override;

    // AudioProcessor interface implementation
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // AudioProcessor implementation - Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // AudioProcessor implementation - Plugin info
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // AudioProcessor implementation - Programs/Presets
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    // AudioProcessor implementation - State
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter access
    juce::AudioProcessorValueTreeState& getValueTreeState() { return m_valueTreeState; }
    const juce::AudioProcessorValueTreeState& getValueTreeState() const { return m_valueTreeState; }
    
    // Preset management
    PresetManager& getPresetManager() { return m_presetManager; }
    const PresetManager& getPresetManager() const { return m_presetManager; }
    
    // Reverb engine access (for editor)
    ReverbEngine& getReverbEngine() { return m_reverbEngine; }
    const ReverbEngine& getReverbEngine() const { return m_reverbEngine; }
    
    // Performance monitoring
    float getCurrentCPUUsage() const { return m_cpuUsage; }
    float getPeakInputLevel() const { return m_peakInputLevel; }
    float getPeakOutputLevel() const { return m_peakOutputLevel; }
    
private:
    // Core components
    ReverbEngine m_reverbEngine;
    juce::AudioProcessorValueTreeState m_valueTreeState;
    PresetManager m_presetManager;
    
    // Parameter atomic references for real-time access
    std::atomic<float>* m_roomSizeParam = nullptr;
    std::atomic<float>* m_decayTimeParam = nullptr;
    std::atomic<float>* m_preDelayParam = nullptr;
    std::atomic<float>* m_dampingParam = nullptr;
    std::atomic<float>* m_wetDryMixParam = nullptr;
    std::atomic<float>* m_earlyLateBalanceParam = nullptr;
    std::atomic<float>* m_diffusionParam = nullptr;
    std::atomic<float>* m_modulationRateParam = nullptr;
    std::atomic<float>* m_modulationDepthParam = nullptr;
    
    // Performance monitoring
    juce::Time m_lastCPUMeasurement;
    float m_cpuUsage = 0.0f;
    std::atomic<float> m_peakInputLevel { 0.0f };
    std::atomic<float> m_peakOutputLevel { 0.0f };
    juce::LinearSmoothedValue<float> m_inputLevelSmoother;
    juce::LinearSmoothedValue<float> m_outputLevelSmoother;
    
    // Audio processing state
    double m_sampleRate = 44100.0;
    int m_samplesPerBlock = 512;
    bool m_isPrepared = false;
    
    // Bypass functionality
    juce::dsp::DryWetMixer<float> m_dryWetMixer;
    bool m_wasBypassed = false;
    
    /**
     * @brief Update reverb engine parameters from APVTS
     */
    void updateReverbParameters();
    
    /**
     * @brief Update performance metrics
     * @param startTime Time when processing started
     */
    void updatePerformanceMetrics(juce::Time startTime);
    
    /**
     * @brief Update level meters
     * @param buffer Audio buffer to analyze
     */
    void updateLevelMeters(const juce::AudioBuffer<float>& buffer);
    
    /**
     * @brief Handle parameter changes from the value tree
     */
    void parameterChanged(const juce::String& parameterID, float newValue);
    
    /**
     * @brief Initialize parameter atomic references
     */
    void initializeParameterReferences();
    
    /**
     * @brief Create parameter layout for APVTS
     * @return Parameter layout
     */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbPluginAudioProcessor)
};

/**
 * @brief Audio processor listener for parameter changes
 * 
 * Handles real-time parameter updates and ensures smooth transitions.
 */
class ReverbParameterListener : public juce::AudioProcessorValueTreeState::Listener
{
public:
    ReverbParameterListener(ReverbPluginAudioProcessor& processor);
    ~ReverbParameterListener() override;
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
private:
    ReverbPluginAudioProcessor& m_processor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbParameterListener)
};