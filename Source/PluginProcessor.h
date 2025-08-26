#pragma once

#include <JuceHeader.h>
#include "ReverbEngine.h"

/**
 * @brief Simplified audio processor for the Reverb Plugin
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
    
    // Reverb engine access (for editor)
    ReverbEngine& getReverbEngine() { return m_reverbEngine; }
    const ReverbEngine& getReverbEngine() const { return m_reverbEngine; }
    
private:
    // Core components
    ReverbEngine m_reverbEngine;
    juce::AudioProcessorValueTreeState m_valueTreeState;
    
    // Parameter atomic references for real-time access
    std::atomic<float>* m_roomSizeParam = nullptr;
    std::atomic<float>* m_decayTimeParam = nullptr;
    std::atomic<float>* m_preDelayParam = nullptr;
    std::atomic<float>* m_dampingParam = nullptr;
    std::atomic<float>* m_wetDryMixParam = nullptr;
    
    // Audio processing state
    double m_sampleRate = 44100.0;
    int m_samplesPerBlock = 512;
    bool m_isPrepared = false;
    
    /**
     * @brief Update reverb engine parameters from APVTS
     */
    void updateReverbParameters();
    
    /**
     * @brief Initialize parameter atomic references
     */
    void initializeParameterReferences();
    
    /**
     * @brief Create parameter layout for APVTS
     */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbPluginAudioProcessor)
};