#include "PluginEditor_Fixed.h"

// ReverbPluginAudioProcessorEditor implementation
ReverbPluginAudioProcessorEditor::ReverbPluginAudioProcessorEditor(ReverbPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , m_audioProcessor(p)
{
    // Set window size
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Initialize components
    initializeComponents();
    setupAttachments();
    
    // Start timer for updates
    startTimerHz(30); // 30 FPS
}

ReverbPluginAudioProcessorEditor::~ReverbPluginAudioProcessorEditor()
{
    stopTimer();
}

void ReverbPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Plugin title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("Reverb Plugin", 20, 10, 200, 30, juce::Justification::left);
}

void ReverbPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50); // Title space
    bounds.reduce(MARGIN, MARGIN);
    
    // Arrange controls in a row
    auto controlWidth = (bounds.getWidth() - (MARGIN * 4)) / 5;
    auto controlArea = bounds.removeFromTop(CONTROL_HEIGHT);
    
    auto setupControl = [&](juce::Slider* slider, juce::Label* label) {
        if (slider && label)
        {
            auto area = controlArea.removeFromLeft(controlWidth);
            label->setBounds(area.removeFromTop(LABEL_HEIGHT));
            slider->setBounds(area.reduced(5));
            controlArea.removeFromLeft(MARGIN);
        }
    };
    
    setupControl(m_roomSizeSlider.get(), m_roomSizeLabel.get());
    setupControl(m_decayTimeSlider.get(), m_decayTimeLabel.get());
    setupControl(m_preDelaySlider.get(), m_preDelayLabel.get());
    setupControl(m_dampingSlider.get(), m_dampingLabel.get());
    setupControl(m_wetDryMixSlider.get(), m_wetDryMixLabel.get());
}

void ReverbPluginAudioProcessorEditor::timerCallback()
{
    // Update any real-time displays here
    repaint();
}

void ReverbPluginAudioProcessorEditor::initializeComponents()
{
    // Create main parameter controls
    m_roomSizeSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_roomSizeSlider);
    addAndMakeVisible(*m_roomSizeSlider);
    
    m_decayTimeSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_decayTimeSlider);
    addAndMakeVisible(*m_decayTimeSlider);
    
    m_preDelaySlider = std::make_unique<juce::Slider>();
    configureSlider(*m_preDelaySlider);
    addAndMakeVisible(*m_preDelaySlider);
    
    m_dampingSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_dampingSlider);
    addAndMakeVisible(*m_dampingSlider);
    
    m_wetDryMixSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_wetDryMixSlider);
    addAndMakeVisible(*m_wetDryMixSlider);
    
    // Create labels
    m_roomSizeLabel = std::make_unique<juce::Label>();
    configureLabel(*m_roomSizeLabel, "Room Size");
    addAndMakeVisible(*m_roomSizeLabel);
    
    m_decayTimeLabel = std::make_unique<juce::Label>();
    configureLabel(*m_decayTimeLabel, "Decay Time");
    addAndMakeVisible(*m_decayTimeLabel);
    
    m_preDelayLabel = std::make_unique<juce::Label>();
    configureLabel(*m_preDelayLabel, "Pre-Delay");
    addAndMakeVisible(*m_preDelayLabel);
    
    m_dampingLabel = std::make_unique<juce::Label>();
    configureLabel(*m_dampingLabel, "Damping");
    addAndMakeVisible(*m_dampingLabel);
    
    m_wetDryMixLabel = std::make_unique<juce::Label>();
    configureLabel(*m_wetDryMixLabel, "Wet/Dry Mix");
    addAndMakeVisible(*m_wetDryMixLabel);
}

void ReverbPluginAudioProcessorEditor::setupAttachments()
{
    auto& apvts = m_audioProcessor.getValueTreeState();
    
    m_roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "roomSize", *m_roomSizeSlider);
    
    m_decayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "decayTime", *m_decayTimeSlider);
    
    m_preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "preDelay", *m_preDelaySlider);
    
    m_dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "damping", *m_dampingSlider);
    
    m_wetDryMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "wetDryMix", *m_wetDryMixSlider);
}

void ReverbPluginAudioProcessorEditor::configureSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff4a9eff));
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4a9eff));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff808080));
}

void ReverbPluginAudioProcessorEditor::configureLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setFont(juce::Font(12.0f));
}