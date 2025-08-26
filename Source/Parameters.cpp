#include "Parameters.h"

namespace ReverbParams
{
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
        
        // Room Size parameter
        auto roomSizeAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 1);
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ROOM_SIZE_ID, 1),
            ROOM_SIZE_NAME,
            juce::NormalisableRange<float>(ROOM_SIZE_MIN, ROOM_SIZE_MAX, 0.1f),
            ROOM_SIZE_DEFAULT,
            roomSizeAttributes
        ));
        
        // Decay Time parameter
        auto decayTimeAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("s")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 1) + " s";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(DECAY_TIME_ID, 1),
            DECAY_TIME_NAME,
            juce::NormalisableRange<float>(DECAY_TIME_MIN, DECAY_TIME_MAX, 0.1f),
            DECAY_TIME_DEFAULT,
            decayTimeAttributes
        ));
        
        // Pre-Delay parameter
        auto preDelayAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("ms")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 0) + " ms";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(PRE_DELAY_ID, 1),
            PRE_DELAY_NAME,
            juce::NormalisableRange<float>(PRE_DELAY_MIN, PRE_DELAY_MAX, 1.0f),
            PRE_DELAY_DEFAULT,
            preDelayAttributes
        ));
        
        // Damping parameter
        auto dampingAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("%")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 0) + "%";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(DAMPING_ID, 1),
            DAMPING_NAME,
            juce::NormalisableRange<float>(DAMPING_MIN, DAMPING_MAX, 1.0f),
            DAMPING_DEFAULT,
            dampingAttributes
        ));
        
        // Wet/Dry Mix parameter
        auto wetDryMixAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("%")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 0) + "%";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(WET_DRY_MIX_ID, 1),
            WET_DRY_MIX_NAME,
            juce::NormalisableRange<float>(WET_DRY_MIX_MIN, WET_DRY_MIX_MAX, 1.0f),
            WET_DRY_MIX_DEFAULT,
            wetDryMixAttributes
        ));
        
        // Early/Late Balance parameter
        auto earlyLateBalanceAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("%")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 0) + "%";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(EARLY_LATE_BALANCE_ID, 1),
            EARLY_LATE_BALANCE_NAME,
            juce::NormalisableRange<float>(EARLY_LATE_BALANCE_MIN, EARLY_LATE_BALANCE_MAX, 1.0f),
            EARLY_LATE_BALANCE_DEFAULT,
            earlyLateBalanceAttributes
        ));
        
        // Diffusion parameter
        auto diffusionAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("%")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 0) + "%";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(DIFFUSION_ID, 1),
            DIFFUSION_NAME,
            juce::NormalisableRange<float>(DIFFUSION_MIN, DIFFUSION_MAX, 1.0f),
            DIFFUSION_DEFAULT,
            diffusionAttributes
        ));
        
        // Modulation Rate parameter
        auto modulationRateAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("Hz")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 1) + " Hz";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(MODULATION_RATE_ID, 1),
            MODULATION_RATE_NAME,
            juce::NormalisableRange<float>(MODULATION_RATE_MIN, MODULATION_RATE_MAX, 0.1f),
            MODULATION_RATE_DEFAULT,
            modulationRateAttributes
        ));
        
        // Modulation Depth parameter
        auto modulationDepthAttributes = juce::AudioParameterFloatAttributes()
            .withCategory(juce::AudioProcessorParameter::genericParameter)
            .withLabel("%")
            .withStringFromValueFunction([](float value, int) {
                return juce::String(value, 0) + "%";
            })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
            });
            
        parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(MODULATION_DEPTH_ID, 1),
            MODULATION_DEPTH_NAME,
            juce::NormalisableRange<float>(MODULATION_DEPTH_MIN, MODULATION_DEPTH_MAX, 1.0f),
            MODULATION_DEPTH_DEFAULT,
            modulationDepthAttributes
        ));
        
        return { parameters.begin(), parameters.end() };
    }
    
    // ReverbParameter implementation
    ReverbParameter::ReverbParameter(const juce::String& parameterID, const juce::String& parameterName,
                                   float minValue, float maxValue, float defaultValue,
                                   const juce::String& labelSuffix)
        : m_parameterID(parameterID), m_parameterName(parameterName),
          m_labelSuffix(labelSuffix), m_range(minValue, maxValue),
          m_value(defaultValue), m_defaultValue(defaultValue)
    {
    }
    
    float ReverbParameter::getValue() const
    {
        return m_range.convertTo0to1(m_value.load());
    }
    
    void ReverbParameter::setValue(float newValue)
    {
        m_value.store(m_range.convertFrom0to1(newValue));
    }
    
    float ReverbParameter::getDefaultValue() const
    {
        return m_range.convertTo0to1(m_defaultValue);
    }
    
    juce::String ReverbParameter::getName(int maximumStringLength) const
    {
        return m_parameterName.substring(0, maximumStringLength);
    }
    
    juce::String ReverbParameter::getLabel() const
    {
        return m_labelSuffix;
    }
    
    float ReverbParameter::getValueForText(const juce::String& text) const
    {
        return m_range.convertTo0to1(text.getFloatValue());
    }
    
    juce::String ReverbParameter::getText(float value, int maximumStringLength) const
    {
        auto denormalisedValue = m_range.convertFrom0to1(value);
        juce::String text = juce::String(denormalisedValue, 2) + m_labelSuffix;
        return text.substring(0, maximumStringLength);
    }
}