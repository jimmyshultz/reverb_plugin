#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 * @brief Simplified GUI editor for the Reverb Plugin
 * 
 * A basic but functional interface for controlling reverb parameters.
 */
class ReverbPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Timer
{
public:
    ReverbPluginAudioProcessorEditor(ReverbPluginAudioProcessor&);
    ~ReverbPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    // Timer callback for updating meters
    void timerCallback() override;

private:
    ReverbPluginAudioProcessor& m_audioProcessor;
    
    // Main parameter controls
    std::unique_ptr<juce::Slider> m_roomSizeSlider;
    std::unique_ptr<juce::Slider> m_decayTimeSlider;
    std::unique_ptr<juce::Slider> m_preDelaySlider;
    std::unique_ptr<juce::Slider> m_dampingSlider;
    std::unique_ptr<juce::Slider> m_wetDryMixSlider;
    
    // Parameter labels
    std::unique_ptr<juce::Label> m_roomSizeLabel;
    std::unique_ptr<juce::Label> m_decayTimeLabel;
    std::unique_ptr<juce::Label> m_preDelayLabel;
    std::unique_ptr<juce::Label> m_dampingLabel;
    std::unique_ptr<juce::Label> m_wetDryMixLabel;
    
    // APVTS attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_decayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_preDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_wetDryMixAttachment;
    
    // Layout constants
    static constexpr int WINDOW_WIDTH = 500;
    static constexpr int WINDOW_HEIGHT = 300;
    static constexpr int MARGIN = 10;
    static constexpr int CONTROL_HEIGHT = 80;
    static constexpr int SLIDER_WIDTH = 80;
    static constexpr int LABEL_HEIGHT = 20;
    
    /**
     * @brief Initialize all GUI components
     */
    void initializeComponents();
    
    /**
     * @brief Setup parameter attachments
     */
    void setupAttachments();
    
    /**
     * @brief Configure slider appearance and properties
     */
    void configureSlider(juce::Slider& slider);
    
    /**
     * @brief Configure label appearance
     */
    void configureLabel(juce::Label& label, const juce::String& text);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbPluginAudioProcessorEditor)
};