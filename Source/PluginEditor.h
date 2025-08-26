#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 * @brief Main GUI editor for the Reverb Plugin
 * 
 * Provides a modern, professional interface for controlling all
 * reverb parameters with visual feedback and preset management.
 */
class ReverbPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Timer
{
public:
    ReverbPluginAudioProcessorEditor(ReverbPluginAudioProcessor&);
    ~ReverbPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    // Timer callback for updating meters and visual feedback
    void timerCallback() override;

private:
    ReverbPluginAudioProcessor& m_audioProcessor;
    
    // Main sections
    std::unique_ptr<juce::GroupComponent> m_mainControlsGroup;
    std::unique_ptr<juce::GroupComponent> m_advancedControlsGroup;
    std::unique_ptr<juce::GroupComponent> m_presetGroup;
    std::unique_ptr<juce::GroupComponent> m_metersGroup;
    
    // Main parameter controls
    std::unique_ptr<juce::Slider> m_roomSizeSlider;
    std::unique_ptr<juce::Slider> m_decayTimeSlider;
    std::unique_ptr<juce::Slider> m_preDelaySlider;
    std::unique_ptr<juce::Slider> m_dampingSlider;
    std::unique_ptr<juce::Slider> m_wetDryMixSlider;
    
    // Advanced parameter controls
    std::unique_ptr<juce::Slider> m_earlyLateBalanceSlider;
    std::unique_ptr<juce::Slider> m_diffusionSlider;
    std::unique_ptr<juce::Slider> m_modulationRateSlider;
    std::unique_ptr<juce::Slider> m_modulationDepthSlider;
    
    // Parameter labels
    std::unique_ptr<juce::Label> m_roomSizeLabel;
    std::unique_ptr<juce::Label> m_decayTimeLabel;
    std::unique_ptr<juce::Label> m_preDelayLabel;
    std::unique_ptr<juce::Label> m_dampingLabel;
    std::unique_ptr<juce::Label> m_wetDryMixLabel;
    std::unique_ptr<juce::Label> m_earlyLateBalanceLabel;
    std::unique_ptr<juce::Label> m_diffusionLabel;
    std::unique_ptr<juce::Label> m_modulationRateLabel;
    std::unique_ptr<juce::Label> m_modulationDepthLabel;
    
    // Preset controls
    std::unique_ptr<juce::ComboBox> m_presetComboBox;
    std::unique_ptr<juce::TextButton> m_savePresetButton;
    std::unique_ptr<juce::TextButton> m_deletePresetButton;
    std::unique_ptr<juce::TextButton> m_previousPresetButton;
    std::unique_ptr<juce::TextButton> m_nextPresetButton;
    
    // Level meters
    std::unique_ptr<juce::Component> m_inputMeter;
    std::unique_ptr<juce::Component> m_outputMeter;
    std::unique_ptr<juce::Label> m_inputMeterLabel;
    std::unique_ptr<juce::Label> m_outputMeterLabel;
    
    // Performance display
    std::unique_ptr<juce::Label> m_cpuUsageLabel;
    
    // APVTS attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_roomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_decayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_preDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_wetDryMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_earlyLateBalanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_diffusionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_modulationRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_modulationDepthAttachment;
    
    // UI state
    bool m_advancedControlsVisible;
    
    // Layout constants
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;
    static constexpr int MARGIN = 10;
    static constexpr int CONTROL_HEIGHT = 80;
    static constexpr int SLIDER_WIDTH = 120;
    static constexpr int LABEL_HEIGHT = 20;
    static constexpr int BUTTON_HEIGHT = 30;
    static constexpr int METER_WIDTH = 20;
    
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
     * @param slider Slider to configure
     * @param style Slider style
     * @param textBoxPosition Position of text box
     */
    void configureSlider(juce::Slider& slider, 
                        juce::Slider::SliderStyle style = juce::Slider::RotaryHorizontalVerticalDrag,
                        juce::Slider::TextEntryBoxPosition textBoxPosition = juce::Slider::TextBoxBelow);
    
    /**
     * @brief Configure label appearance
     * @param label Label to configure
     * @param text Label text
     * @param justification Text justification
     */
    void configureLabel(juce::Label& label, 
                       const juce::String& text,
                       juce::Justification justification = juce::Justification::centred);
    
    /**
     * @brief Setup preset controls
     */
    void setupPresetControls();
    
    /**
     * @brief Update preset combo box with available presets
     */
    void updatePresetComboBox();
    
    /**
     * @brief Handle preset selection changes
     */
    void onPresetChanged();
    
    /**
     * @brief Show save preset dialog
     */
    void showSavePresetDialog();
    
    /**
     * @brief Handle delete preset button
     */
    void onDeletePreset();
    
    /**
     * @brief Navigate to previous preset
     */
    void navigateToPreviousPreset();
    
    /**
     * @brief Navigate to next preset
     */
    void navigateToNextPreset();
    
    /**
     * @brief Toggle advanced controls visibility
     */
    void toggleAdvancedControls();
    
    /**
     * @brief Update level meters
     */
    void updateLevelMeters();
    
    /**
     * @brief Update CPU usage display
     */
    void updateCPUUsage();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbPluginAudioProcessorEditor)
};

/**
 * @brief Custom level meter component
 * 
 * Displays audio level with peak hold and smooth decay.
 */
class LevelMeter : public juce::Component
{
public:
    LevelMeter();
    ~LevelMeter() override = default;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    /**
     * @brief Set the current level
     * @param level Level value (0.0 to 1.0)
     */
    void setLevel(float level);
    
    /**
     * @brief Set meter orientation
     * @param vertical True for vertical meter, false for horizontal
     */
    void setVertical(bool vertical) { m_isVertical = vertical; }
    
private:
    float m_level;
    float m_peakLevel;
    float m_peakHoldTime;
    bool m_isVertical;
    
    juce::Colour m_lowColour;
    juce::Colour m_midColour;
    juce::Colour m_highColour;
    juce::Colour m_peakColour;
    
    static constexpr float PEAK_HOLD_TIME = 1.0f; // seconds
    static constexpr float DECAY_RATE = 0.95f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

/**
 * @brief Custom rotary slider with enhanced appearance
 * 
 * Provides a modern look for parameter controls with
 * value display and parameter name.
 */
class CustomRotarySlider : public juce::Component
{
public:
    CustomRotarySlider(const juce::String& parameterName);
    ~CustomRotarySlider() override = default;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    /**
     * @brief Set the value range
     * @param minimum Minimum value
     * @param maximum Maximum value
     * @param interval Step interval
     */
    void setRange(double minimum, double maximum, double interval = 0.01);
    
    /**
     * @brief Set the current value
     * @param value New value
     * @param sendNotification Whether to send change notification
     */
    void setValue(double value, bool sendNotification = true);
    
    /**
     * @brief Get the current value
     * @return Current value
     */
    double getValue() const { return m_value; }
    
    /**
     * @brief Set value to text conversion function
     * @param func Function to convert value to display text
     */
    void setTextFromValueFunction(std::function<juce::String(double)> func);
    
    /**
     * @brief Set text to value conversion function
     * @param func Function to convert text to value
     */
    void setValueFromTextFunction(std::function<double(const juce::String&)> func);
    
    /**
     * @brief Set callback for value changes
     * @param callback Function to call when value changes
     */
    void onValueChange(std::function<void()> callback);
    
private:
    juce::String m_parameterName;
    double m_value;
    double m_minimum;
    double m_maximum;
    double m_interval;
    bool m_isDragging;
    int m_lastMouseY;
    
    std::function<juce::String(double)> m_textFromValueFunction;
    std::function<double(const juce::String&)> m_valueFromTextFunction;
    std::function<void()> m_valueChangeCallback;
    
    /**
     * @brief Convert value to angle for drawing
     * @param value Value to convert
     * @return Angle in radians
     */
    float valueToAngle(double value) const;
    
    /**
     * @brief Convert angle to value
     * @param angle Angle in radians
     * @return Corresponding value
     */
    double angleToValue(float angle) const;
    
    /**
     * @brief Get display text for current value
     * @return Formatted text
     */
    juce::String getDisplayText() const;
    
    static constexpr float START_ANGLE = juce::MathConstants<float>::pi * 1.25f;
    static constexpr float END_ANGLE = juce::MathConstants<float>::pi * 2.75f;
    static constexpr float SENSITIVITY = 0.005f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomRotarySlider)
};

/**
 * @brief Modern look and feel for the plugin
 * 
 * Provides a custom appearance for all GUI components
 * with a professional, modern aesthetic.
 */
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();
    ~ModernLookAndFeel() override = default;
    
    // Slider appearance
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;
    
    // Button appearance
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
    
    // ComboBox appearance
    void drawComboBox(juce::Graphics& g, int width, int height,
                     bool isButtonDown, int buttonX, int buttonY,
                     int buttonW, int buttonH, juce::ComboBox& box) override;
    
    // Label appearance
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    
    // GroupComponent appearance
    void drawGroupComponentOutline(juce::Graphics& g, int width, int height,
                                 const juce::String& text,
                                 const juce::Justification& position,
                                 juce::GroupComponent& group) override;
    
private:
    juce::Colour m_primaryColour;
    juce::Colour m_secondaryColour;
    juce::Colour m_accentColour;
    juce::Colour m_backgroundColour;
    juce::Colour m_textColour;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernLookAndFeel)
};