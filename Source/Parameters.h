#pragma once

#include <JuceHeader.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

/**
 * @brief Parameter identifiers and definitions for the Reverb Plugin
 * 
 * This file contains all parameter IDs, names, ranges, and default values
 * used throughout the plugin for the Audio Processor Value Tree State (APVTS).
 */

namespace ReverbParams
{
    // Parameter IDs
    const juce::String ROOM_SIZE_ID = "roomSize";
    const juce::String DECAY_TIME_ID = "decayTime";
    const juce::String PRE_DELAY_ID = "preDelay";
    const juce::String DAMPING_ID = "damping";
    const juce::String WET_DRY_MIX_ID = "wetDryMix";
    const juce::String EARLY_LATE_BALANCE_ID = "earlyLateBalance";
    const juce::String DIFFUSION_ID = "diffusion";
    const juce::String MODULATION_RATE_ID = "modulationRate";
    const juce::String MODULATION_DEPTH_ID = "modulationDepth";
    
    // Parameter display names
    const juce::String ROOM_SIZE_NAME = "Room Size";
    const juce::String DECAY_TIME_NAME = "Decay Time";
    const juce::String PRE_DELAY_NAME = "Pre-Delay";
    const juce::String DAMPING_NAME = "Damping";
    const juce::String WET_DRY_MIX_NAME = "Wet/Dry Mix";
    const juce::String EARLY_LATE_BALANCE_NAME = "Early/Late Balance";
    const juce::String DIFFUSION_NAME = "Diffusion";
    const juce::String MODULATION_RATE_NAME = "Modulation Rate";
    const juce::String MODULATION_DEPTH_NAME = "Modulation Depth";
    
    // Parameter ranges and defaults
    const float ROOM_SIZE_MIN = 0.1f;
    const float ROOM_SIZE_MAX = 10.0f;
    const float ROOM_SIZE_DEFAULT = 1.0f;
    
    const float DECAY_TIME_MIN = 0.1f;
    const float DECAY_TIME_MAX = 20.0f;
    const float DECAY_TIME_DEFAULT = 2.0f;
    
    const float PRE_DELAY_MIN = 0.0f;
    const float PRE_DELAY_MAX = 500.0f;
    const float PRE_DELAY_DEFAULT = 20.0f;
    
    const float DAMPING_MIN = 0.0f;
    const float DAMPING_MAX = 100.0f;
    const float DAMPING_DEFAULT = 50.0f;
    
    const float WET_DRY_MIX_MIN = 0.0f;
    const float WET_DRY_MIX_MAX = 100.0f;
    const float WET_DRY_MIX_DEFAULT = 30.0f;
    
    const float EARLY_LATE_BALANCE_MIN = 0.0f;
    const float EARLY_LATE_BALANCE_MAX = 100.0f;
    const float EARLY_LATE_BALANCE_DEFAULT = 50.0f;
    
    const float DIFFUSION_MIN = 0.0f;
    const float DIFFUSION_MAX = 100.0f;
    const float DIFFUSION_DEFAULT = 75.0f;
    
    const float MODULATION_RATE_MIN = 0.1f;
    const float MODULATION_RATE_MAX = 5.0f;
    const float MODULATION_RATE_DEFAULT = 1.0f;
    
    const float MODULATION_DEPTH_MIN = 0.0f;
    const float MODULATION_DEPTH_MAX = 100.0f;
    const float MODULATION_DEPTH_DEFAULT = 25.0f;
    
    /**
     * @brief Creates the AudioProcessorValueTreeState::ParameterLayout for the reverb plugin
     * @return ParameterLayout containing all plugin parameters with their ranges and defaults
     */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    /**
     * @brief Custom parameter classes for specific parameter types
     */
    class ReverbParameter : public juce::RangedAudioParameter
    {
    public:
        ReverbParameter(const juce::String& parameterID, const juce::String& parameterName,
                       float minValue, float maxValue, float defaultValue,
                       const juce::String& labelSuffix = "");
        
        float getValue() const override;
        void setValue(float newValue) override;
        float getDefaultValue() const override;
        juce::String getName(int maximumStringLength) const override;
        juce::String getLabel() const override;
        float getValueForText(const juce::String& text) const override;
        juce::String getText(float value, int maximumStringLength) const override;
        
    private:
        juce::String m_parameterID;
        juce::String m_parameterName;
        juce::String m_labelSuffix;
        juce::NormalisableRange<float> m_range;
        std::atomic<float> m_value;
        float m_defaultValue;
    };
}