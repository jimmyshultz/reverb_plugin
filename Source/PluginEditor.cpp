#include "PluginEditor.h"

// ReverbPluginAudioProcessorEditor implementation
ReverbPluginAudioProcessorEditor::ReverbPluginAudioProcessorEditor(ReverbPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , m_audioProcessor(p)
    , m_advancedControlsVisible(false)
{
    // Set window size
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setResizable(true, true);
    setResizeLimits(600, 400, 1200, 800);
    
    // Initialize components
    initializeComponents();
    setupAttachments();
    setupPresetControls();
    
    // Start timer for meters and visual updates
    startTimerHz(30); // 30 FPS
    
    // Set custom look and feel
    setLookAndFeel(&getLookAndFeel());
}

ReverbPluginAudioProcessorEditor::~ReverbPluginAudioProcessorEditor()
{
    stopTimer();
}

void ReverbPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient gradient(juce::Colour(0xff2a2a2a), 0, 0,
                                 juce::Colour(0xff1a1a1a), 0, static_cast<float>(getHeight()),
                                 false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Plugin title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("Reverb Plugin", 20, 10, 200, 30, juce::Justification::left);
    
    // Version info
    g.setFont(juce::Font(12.0f));
    g.setColour(juce::Colours::lightgrey);
    g.drawText("v1.0.0", getWidth() - 100, 10, 80, 20, juce::Justification::right);
}

void ReverbPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50); // Title space
    
    // Preset section
    auto presetArea = bounds.removeFromTop(80);
    presetArea.reduce(MARGIN, MARGIN);
    if (m_presetGroup)
    {
        m_presetGroup->setBounds(presetArea);
        
        auto presetBounds = presetArea.reduced(10);
        presetBounds.removeFromTop(20); // Group title space
        
        auto buttonWidth = 60;
        auto comboWidth = presetBounds.getWidth() - (buttonWidth * 4) - (MARGIN * 5);
        
        if (m_previousPresetButton)
            m_previousPresetButton->setBounds(presetBounds.removeFromLeft(buttonWidth));
        presetBounds.removeFromLeft(MARGIN);
        
        if (m_presetComboBox)
            m_presetComboBox->setBounds(presetBounds.removeFromLeft(comboWidth));
        presetBounds.removeFromLeft(MARGIN);
        
        if (m_nextPresetButton)
            m_nextPresetButton->setBounds(presetBounds.removeFromLeft(buttonWidth));
        presetBounds.removeFromLeft(MARGIN);
        
        if (m_savePresetButton)
            m_savePresetButton->setBounds(presetBounds.removeFromLeft(buttonWidth));
        presetBounds.removeFromLeft(MARGIN);
        
        if (m_deletePresetButton)
            m_deletePresetButton->setBounds(presetBounds.removeFromLeft(buttonWidth));
    }
    
    // Level meters section
    auto metersArea = bounds.removeFromRight(100);
    if (m_metersGroup)
    {
        m_metersGroup->setBounds(metersArea);
        
        auto meterBounds = metersArea.reduced(10);
        meterBounds.removeFromTop(20); // Group title space
        
        auto meterHeight = (meterBounds.getHeight() - MARGIN) / 2;
        
        auto inputArea = meterBounds.removeFromTop(meterHeight);
        if (m_inputMeterLabel)
            m_inputMeterLabel->setBounds(inputArea.removeFromTop(LABEL_HEIGHT));
        if (m_inputMeter)
            m_inputMeter->setBounds(inputArea.reduced(10));
        
        meterBounds.removeFromTop(MARGIN);
        auto outputArea = meterBounds.removeFromTop(meterHeight);
        if (m_outputMeterLabel)
            m_outputMeterLabel->setBounds(outputArea.removeFromTop(LABEL_HEIGHT));
        if (m_outputMeter)
            m_outputMeter->setBounds(outputArea.reduced(10));
        
        // CPU usage at bottom
        if (m_cpuUsageLabel)
            m_cpuUsageLabel->setBounds(meterBounds.removeFromBottom(30));
    }
    
    // Main controls
    auto controlsArea = bounds;
    controlsArea.reduce(MARGIN, MARGIN);
    
    // Main controls section
    auto mainControlsHeight = 200;
    if (m_mainControlsGroup)
    {
        auto mainArea = controlsArea.removeFromTop(mainControlsHeight);
        m_mainControlsGroup->setBounds(mainArea);
        
        auto mainBounds = mainArea.reduced(10);
        mainBounds.removeFromTop(20); // Group title space
        
        // Arrange main controls in a grid
        auto controlWidth = (mainBounds.getWidth() - (MARGIN * 4)) / 5;
        auto controlArea = mainBounds.removeFromTop(CONTROL_HEIGHT);
        
        auto setupMainControl = [&](juce::Slider* slider, juce::Label* label) {
            if (slider && label)
            {
                auto area = controlArea.removeFromLeft(controlWidth);
                label->setBounds(area.removeFromTop(LABEL_HEIGHT));
                slider->setBounds(area.reduced(5));
                controlArea.removeFromLeft(MARGIN);
            }
        };
        
        setupMainControl(m_roomSizeSlider.get(), m_roomSizeLabel.get());
        setupMainControl(m_decayTimeSlider.get(), m_decayTimeLabel.get());
        setupMainControl(m_preDelaySlider.get(), m_preDelayLabel.get());
        setupMainControl(m_dampingSlider.get(), m_dampingLabel.get());
        setupMainControl(m_wetDryMixSlider.get(), m_wetDryMixLabel.get());
    }
    
    controlsArea.removeFromTop(MARGIN);
    
    // Advanced controls section (if visible)
    if (m_advancedControlsVisible && m_advancedControlsGroup)
    {
        auto advancedArea = controlsArea.removeFromTop(200);
        m_advancedControlsGroup->setBounds(advancedArea);
        
        auto advancedBounds = advancedArea.reduced(10);
        advancedBounds.removeFromTop(20); // Group title space
        
        // Arrange advanced controls
        auto controlWidth = (advancedBounds.getWidth() - (MARGIN * 3)) / 4;
        auto controlArea = advancedBounds.removeFromTop(CONTROL_HEIGHT);
        
        auto setupAdvancedControl = [&](juce::Slider* slider, juce::Label* label) {
            if (slider && label)
            {
                auto area = controlArea.removeFromLeft(controlWidth);
                label->setBounds(area.removeFromTop(LABEL_HEIGHT));
                slider->setBounds(area.reduced(5));
                controlArea.removeFromLeft(MARGIN);
            }
        };
        
        setupAdvancedControl(m_earlyLateBalanceSlider.get(), m_earlyLateBalanceLabel.get());
        setupAdvancedControl(m_diffusionSlider.get(), m_diffusionLabel.get());
        setupAdvancedControl(m_modulationRateSlider.get(), m_modulationRateLabel.get());
        setupAdvancedControl(m_modulationDepthSlider.get(), m_modulationDepthLabel.get());
    }
}

void ReverbPluginAudioProcessorEditor::timerCallback()
{
    updateLevelMeters();
    updateCPUUsage();
    
    // Update preset combo box if presets have changed
    updatePresetComboBox();
}

void ReverbPluginAudioProcessorEditor::initializeComponents()
{
    // Create group components
    m_mainControlsGroup = std::make_unique<juce::GroupComponent>("Main", "Main Controls");
    addAndMakeVisible(*m_mainControlsGroup);
    
    m_advancedControlsGroup = std::make_unique<juce::GroupComponent>("Advanced", "Advanced Controls");
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_advancedControlsGroup);
    
    m_presetGroup = std::make_unique<juce::GroupComponent>("Presets", "Presets");
    addAndMakeVisible(*m_presetGroup);
    
    m_metersGroup = std::make_unique<juce::GroupComponent>("Meters", "Levels");
    addAndMakeVisible(*m_metersGroup);
    
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
    
    // Create advanced parameter controls
    m_earlyLateBalanceSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_earlyLateBalanceSlider);
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_earlyLateBalanceSlider);
    
    m_diffusionSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_diffusionSlider);
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_diffusionSlider);
    
    m_modulationRateSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_modulationRateSlider);
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_modulationRateSlider);
    
    m_modulationDepthSlider = std::make_unique<juce::Slider>();
    configureSlider(*m_modulationDepthSlider);
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_modulationDepthSlider);
    
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
    
    m_earlyLateBalanceLabel = std::make_unique<juce::Label>();
    configureLabel(*m_earlyLateBalanceLabel, "Early/Late");
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_earlyLateBalanceLabel);
    
    m_diffusionLabel = std::make_unique<juce::Label>();
    configureLabel(*m_diffusionLabel, "Diffusion");
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_diffusionLabel);
    
    m_modulationRateLabel = std::make_unique<juce::Label>();
    configureLabel(*m_modulationRateLabel, "Mod Rate");
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_modulationRateLabel);
    
    m_modulationDepthLabel = std::make_unique<juce::Label>();
    configureLabel(*m_modulationDepthLabel, "Mod Depth");
    if (m_advancedControlsVisible)
        addAndMakeVisible(*m_modulationDepthLabel);
    
    // Create level meters
    m_inputMeter = std::make_unique<LevelMeter>();
    addAndMakeVisible(*m_inputMeter);
    
    m_outputMeter = std::make_unique<LevelMeter>();
    addAndMakeVisible(*m_outputMeter);
    
    m_inputMeterLabel = std::make_unique<juce::Label>();
    configureLabel(*m_inputMeterLabel, "Input");
    addAndMakeVisible(*m_inputMeterLabel);
    
    m_outputMeterLabel = std::make_unique<juce::Label>();
    configureLabel(*m_outputMeterLabel, "Output");
    addAndMakeVisible(*m_outputMeterLabel);
    
    // CPU usage label
    m_cpuUsageLabel = std::make_unique<juce::Label>();
    configureLabel(*m_cpuUsageLabel, "CPU: 0%", juce::Justification::centred);
    m_cpuUsageLabel->setFont(juce::Font(10.0f));
    addAndMakeVisible(*m_cpuUsageLabel);
}

void ReverbPluginAudioProcessorEditor::setupAttachments()
{
    auto& apvts = m_audioProcessor.getValueTreeState();
    
    m_roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::ROOM_SIZE_ID, *m_roomSizeSlider);
    
    m_decayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::DECAY_TIME_ID, *m_decayTimeSlider);
    
    m_preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::PRE_DELAY_ID, *m_preDelaySlider);
    
    m_dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::DAMPING_ID, *m_dampingSlider);
    
    m_wetDryMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::WET_DRY_MIX_ID, *m_wetDryMixSlider);
    
    m_earlyLateBalanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::EARLY_LATE_BALANCE_ID, *m_earlyLateBalanceSlider);
    
    m_diffusionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::DIFFUSION_ID, *m_diffusionSlider);
    
    m_modulationRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::MODULATION_RATE_ID, *m_modulationRateSlider);
    
    m_modulationDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ReverbParams::MODULATION_DEPTH_ID, *m_modulationDepthSlider);
}

void ReverbPluginAudioProcessorEditor::configureSlider(juce::Slider& slider, 
                                                      juce::Slider::SliderStyle style,
                                                      juce::Slider::TextEntryBoxPosition textBoxPosition)
{
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(textBoxPosition, false, 60, 20);
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff4a9eff));
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4a9eff));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff808080));
}

void ReverbPluginAudioProcessorEditor::configureLabel(juce::Label& label, 
                                                     const juce::String& text,
                                                     juce::Justification justification)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(justification);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setFont(juce::Font(12.0f));
}

void ReverbPluginAudioProcessorEditor::setupPresetControls()
{
    // Preset combo box
    m_presetComboBox = std::make_unique<juce::ComboBox>();
    m_presetComboBox->setTextWhenNothingSelected("Select Preset...");
    m_presetComboBox->onChange = [this] { onPresetChanged(); };
    addAndMakeVisible(*m_presetComboBox);
    
    // Navigation buttons
    m_previousPresetButton = std::make_unique<juce::TextButton>("<");
    m_previousPresetButton->onClick = [this] { navigateToPreviousPreset(); };
    addAndMakeVisible(*m_previousPresetButton);
    
    m_nextPresetButton = std::make_unique<juce::TextButton>(">");
    m_nextPresetButton->onClick = [this] { navigateToNextPreset(); };
    addAndMakeVisible(*m_nextPresetButton);
    
    // Save/Delete buttons
    m_savePresetButton = std::make_unique<juce::TextButton>("Save");
    m_savePresetButton->onClick = [this] { showSavePresetDialog(); };
    addAndMakeVisible(*m_savePresetButton);
    
    m_deletePresetButton = std::make_unique<juce::TextButton>("Delete");
    m_deletePresetButton->onClick = [this] { onDeletePreset(); };
    addAndMakeVisible(*m_deletePresetButton);
    
    updatePresetComboBox();
}

void ReverbPluginAudioProcessorEditor::updatePresetComboBox()
{
    if (!m_presetComboBox)
        return;
    
    m_presetComboBox->clear();
    
    auto& presetManager = m_audioProcessor.getPresetManager();
    for (int i = 0; i < presetManager.getNumPresets(); ++i)
    {
        auto preset = presetManager.getPreset(i);
        juce::String itemText = preset.name;
        if (preset.isFactoryPreset)
            itemText += " (Factory)";
        
        m_presetComboBox->addItem(itemText, i + 1);
    }
    
    // Select current preset
    int currentPreset = presetManager.getCurrentPresetIndex();
    if (currentPreset >= 0)
        m_presetComboBox->setSelectedId(currentPreset + 1, juce::dontSendNotification);
}

void ReverbPluginAudioProcessorEditor::onPresetChanged()
{
    int selectedId = m_presetComboBox->getSelectedId();
    if (selectedId > 0)
    {
        m_audioProcessor.getPresetManager().loadPreset(selectedId - 1);
    }
}

void ReverbPluginAudioProcessorEditor::showSavePresetDialog()
{
    auto options = juce::MessageBoxOptions()
        .withIconType(juce::MessageBoxIconType::QuestionIcon)
        .withTitle("Save Preset")
        .withMessage("Enter a name for the preset:")
        .withButton("Save")
        .withButton("Cancel");
    
    // This is a simplified implementation - in a real plugin you'd want a custom dialog
    juce::AlertWindow::showAsync(options, [this](int result) {
        if (result == 1) // Save button
        {
            juce::String presetName = "New Preset"; // In real implementation, get from text input
            m_audioProcessor.getPresetManager().savePreset(presetName);
            updatePresetComboBox();
        }
    });
}

void ReverbPluginAudioProcessorEditor::onDeletePreset()
{
    auto& presetManager = m_audioProcessor.getPresetManager();
    int currentPreset = presetManager.getCurrentPresetIndex();
    
    if (currentPreset >= 0)
    {
        auto preset = presetManager.getPreset(currentPreset);
        if (!preset.isFactoryPreset && !preset.isReadOnly)
        {
            auto options = juce::MessageBoxOptions()
                .withIconType(juce::MessageBoxIconType::WarningIcon)
                .withTitle("Delete Preset")
                .withMessage("Are you sure you want to delete the preset '" + preset.name + "'?")
                .withButton("Delete")
                .withButton("Cancel");
            
            juce::AlertWindow::showAsync(options, [this, currentPreset](int result) {
                if (result == 1) // Delete button
                {
                    m_audioProcessor.getPresetManager().deletePreset(currentPreset);
                    updatePresetComboBox();
                }
            });
        }
    }
}

void ReverbPluginAudioProcessorEditor::navigateToPreviousPreset()
{
    auto& presetManager = m_audioProcessor.getPresetManager();
    int currentPreset = presetManager.getCurrentPresetIndex();
    int previousPreset = currentPreset > 0 ? currentPreset - 1 : presetManager.getNumPresets() - 1;
    
    presetManager.loadPreset(previousPreset);
    updatePresetComboBox();
}

void ReverbPluginAudioProcessorEditor::navigateToNextPreset()
{
    auto& presetManager = m_audioProcessor.getPresetManager();
    int currentPreset = presetManager.getCurrentPresetIndex();
    int nextPreset = (currentPreset + 1) % presetManager.getNumPresets();
    
    presetManager.loadPreset(nextPreset);
    updatePresetComboBox();
}

void ReverbPluginAudioProcessorEditor::updateLevelMeters()
{
    if (auto* inputMeter = dynamic_cast<LevelMeter*>(m_inputMeter.get()))
    {
        inputMeter->setLevel(m_audioProcessor.getPeakInputLevel());
    }
    
    if (auto* outputMeter = dynamic_cast<LevelMeter*>(m_outputMeter.get()))
    {
        outputMeter->setLevel(m_audioProcessor.getPeakOutputLevel());
    }
}

void ReverbPluginAudioProcessorEditor::updateCPUUsage()
{
    if (m_cpuUsageLabel)
    {
        float cpuUsage = m_audioProcessor.getCurrentCPUUsage();
        juce::String cpuText = "CPU: " + juce::String(cpuUsage, 1) + "%";
        m_cpuUsageLabel->setText(cpuText, juce::dontSendNotification);
        
        // Change color based on CPU usage
        juce::Colour textColour = juce::Colours::white;
        if (cpuUsage > 80.0f)
            textColour = juce::Colours::red;
        else if (cpuUsage > 60.0f)
            textColour = juce::Colours::orange;
        else if (cpuUsage > 40.0f)
            textColour = juce::Colours::yellow;
        
        m_cpuUsageLabel->setColour(juce::Label::textColourId, textColour);
    }
}

// LevelMeter implementation
LevelMeter::LevelMeter()
    : m_level(0.0f)
    , m_peakLevel(0.0f)
    , m_peakHoldTime(0.0f)
    , m_isVertical(true)
    , m_lowColour(juce::Colours::green)
    , m_midColour(juce::Colours::yellow)
    , m_highColour(juce::Colours::red)
    , m_peakColour(juce::Colours::white)
{
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    g.setColour(juce::Colour(0xff202020));
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Level bar
    if (m_level > 0.0f)
    {
        juce::Rectangle<float> levelBounds;
        
        if (m_isVertical)
        {
            float levelHeight = bounds.getHeight() * m_level;
            levelBounds = juce::Rectangle<float>(bounds.getX(), bounds.getBottom() - levelHeight,
                                               bounds.getWidth(), levelHeight);
        }
        else
        {
            float levelWidth = bounds.getWidth() * m_level;
            levelBounds = juce::Rectangle<float>(bounds.getX(), bounds.getY(),
                                               levelWidth, bounds.getHeight());
        }
        
        // Color gradient based on level
        juce::Colour levelColour = m_lowColour;
        if (m_level > 0.8f)
            levelColour = m_highColour;
        else if (m_level > 0.6f)
            levelColour = m_midColour;
        
        g.setColour(levelColour);
        g.fillRoundedRectangle(levelBounds, 2.0f);
    }
    
    // Peak indicator
    if (m_peakLevel > 0.0f && m_peakHoldTime > 0.0f)
    {
        g.setColour(m_peakColour);
        
        if (m_isVertical)
        {
            float peakY = bounds.getBottom() - (bounds.getHeight() * m_peakLevel);
            g.drawLine(bounds.getX(), peakY, bounds.getRight(), peakY, 2.0f);
        }
        else
        {
            float peakX = bounds.getX() + (bounds.getWidth() * m_peakLevel);
            g.drawLine(peakX, bounds.getY(), peakX, bounds.getBottom(), 2.0f);
        }
    }
    
    // Border
    g.setColour(juce::Colour(0xff808080));
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
}

void LevelMeter::resized()
{
    repaint();
}

void LevelMeter::setLevel(float level)
{
    level = juce::jlimit(0.0f, 1.0f, level);
    
    m_level = level;
    
    // Update peak
    if (level > m_peakLevel)
    {
        m_peakLevel = level;
        m_peakHoldTime = PEAK_HOLD_TIME;
    }
    else
    {
        m_peakLevel *= DECAY_RATE;
        m_peakHoldTime -= 1.0f / 30.0f; // Assuming 30 FPS updates
        
        if (m_peakHoldTime <= 0.0f)
            m_peakLevel = level;
    }
    
    repaint();
}

// Dummy implementation for ModernLookAndFeel and CustomRotarySlider
// These would be fully implemented in a complete plugin

class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel() = default;
    ~ModernLookAndFeel() override = default;
    
    // Implementation would go here for custom appearance
};

// Create a global instance to use in the editor
static ModernLookAndFeel modernLookAndFeel;