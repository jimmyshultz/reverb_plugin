#include "PresetManager.h"
#include <algorithm>

// PresetData implementation
PresetManager::PresetData::PresetData()
    : name("Untitled")
    , author("")
    , description("")
    , category("User")
    , dateCreated(juce::Time::getCurrentTime())
    , dateModified(juce::Time::getCurrentTime())
    , isFactoryPreset(false)
    , isReadOnly(false)
{
}

PresetManager::PresetData::PresetData(const juce::String& presetName)
    : name(presetName)
    , author("")
    , description("")
    , category("User")
    , dateCreated(juce::Time::getCurrentTime())
    , dateModified(juce::Time::getCurrentTime())
    , isFactoryPreset(false)
    , isReadOnly(false)
{
}

// PresetManager implementation
PresetManager::PresetManager(juce::AudioProcessorValueTreeState& valueTreeState)
    : m_valueTreeState(valueTreeState)
    , m_currentPresetIndex(-1)
{
}

PresetManager::~PresetManager()
{
    m_fileWatcher.reset();
}

void PresetManager::initialize()
{
    m_presets.clear();
    
    // Load factory presets first
    loadFactoryPresets();
    
    // Load user presets
    loadUserPresets();
    
    // Sort presets
    sortPresets();
    
    // Set up file system watcher for user preset directory
    auto presetDir = getPresetsDirectory();
    if (presetDir.exists())
    {
        m_fileWatcher = std::make_unique<juce::FileSystemWatcher>();
        m_fileWatcher->addFolder(presetDir);
        m_fileWatcher->addListener([this] { onFileSystemChange(); });
    }
}

int PresetManager::getNumPresets() const
{
    return static_cast<int>(m_presets.size());
}

PresetManager::PresetData PresetManager::getPreset(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_presets.size()))
        return m_presets[static_cast<size_t>(index)];
    
    return PresetData();
}

juce::String PresetManager::getPresetName(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_presets.size()))
        return m_presets[static_cast<size_t>(index)].name;
    
    return "Unknown";
}

bool PresetManager::loadPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(m_presets.size()))
        return false;
    
    const auto& preset = m_presets[static_cast<size_t>(index)];
    
    // Apply preset parameters to value tree state
    applyPresetToValueTree(preset);
    
    m_currentPresetIndex = index;
    
    // Notify callback
    if (m_presetChangeCallback)
        m_presetChangeCallback(index);
    
    return true;
}

bool PresetManager::loadPreset(const juce::String& name)
{
    for (size_t i = 0; i < m_presets.size(); ++i)
    {
        if (m_presets[i].name == name)
        {
            return loadPreset(static_cast<int>(i));
        }
    }
    
    return false;
}

bool PresetManager::savePreset(const juce::String& name,
                              const juce::String& author,
                              const juce::String& description,
                              const juce::String& category)
{
    if (name.isEmpty())
        return false;
    
    // Create preset from current state
    auto preset = createPresetFromCurrentState(name, author, description, category);
    
    // Check if preset with same name already exists
    for (size_t i = 0; i < m_presets.size(); ++i)
    {
        if (m_presets[i].name == name && !m_presets[i].isFactoryPreset)
        {
            // Update existing preset
            m_presets[i] = preset;
            m_presets[i].dateModified = juce::Time::getCurrentTime();
            
            // Save to file
            auto presetFile = getPresetsDirectory().getChildFile(generateUniquePresetFilename(name));
            auto xml = savePresetToXml(preset);
            if (xml && xml->writeTo(presetFile))
            {
                m_currentPresetIndex = static_cast<int>(i);
                if (m_presetChangeCallback)
                    m_presetChangeCallback(m_currentPresetIndex);
                return true;
            }
            return false;
        }
    }
    
    // Add new preset
    m_presets.push_back(preset);
    
    // Save to file
    auto presetFile = getPresetsDirectory().getChildFile(generateUniquePresetFilename(name));
    presetFile.getParentDirectory().createDirectory();
    
    auto xml = savePresetToXml(preset);
    if (xml && xml->writeTo(presetFile))
    {
        m_currentPresetIndex = static_cast<int>(m_presets.size() - 1);
        sortPresets();
        
        if (m_presetChangeCallback)
            m_presetChangeCallback(m_currentPresetIndex);
        
        return true;
    }
    
    // Remove from memory if file save failed
    m_presets.pop_back();
    return false;
}

bool PresetManager::deletePreset(int index)
{
    if (index < 0 || index >= static_cast<int>(m_presets.size()))
        return false;
    
    const auto& preset = m_presets[static_cast<size_t>(index)];
    
    // Can't delete factory presets
    if (preset.isFactoryPreset || preset.isReadOnly)
        return false;
    
    // Delete file
    auto presetFile = getPresetsDirectory().getChildFile(generateUniquePresetFilename(preset.name));
    if (presetFile.exists())
        presetFile.deleteFile();
    
    // Remove from memory
    m_presets.erase(m_presets.begin() + index);
    
    // Update current preset index
    if (m_currentPresetIndex == index)
        m_currentPresetIndex = -1;
    else if (m_currentPresetIndex > index)
        m_currentPresetIndex--;
    
    if (m_presetChangeCallback)
        m_presetChangeCallback(m_currentPresetIndex);
    
    return true;
}

bool PresetManager::renamePreset(int index, const juce::String& newName)
{
    if (index < 0 || index >= static_cast<int>(m_presets.size()) || newName.isEmpty())
        return false;
    
    auto& preset = m_presets[static_cast<size_t>(index)];
    
    // Can't rename factory presets
    if (preset.isFactoryPreset || preset.isReadOnly)
        return false;
    
    // Delete old file
    auto oldFile = getPresetsDirectory().getChildFile(generateUniquePresetFilename(preset.name));
    if (oldFile.exists())
        oldFile.deleteFile();
    
    // Update preset name
    preset.name = newName;
    preset.dateModified = juce::Time::getCurrentTime();
    
    // Save with new name
    auto newFile = getPresetsDirectory().getChildFile(generateUniquePresetFilename(newName));
    auto xml = savePresetToXml(preset);
    
    if (xml && xml->writeTo(newFile))
    {
        sortPresets();
        
        if (m_presetChangeCallback)
            m_presetChangeCallback(m_currentPresetIndex);
        
        return true;
    }
    
    return false;
}

int PresetManager::findMatchingPreset() const
{
    auto currentState = createPresetFromCurrentState("temp", "", "", "");
    
    for (size_t i = 0; i < m_presets.size(); ++i)
    {
        if (presetsMatch(currentState, m_presets[i]))
            return static_cast<int>(i);
    }
    
    return -1;
}

std::vector<int> PresetManager::getPresetsInCategory(const juce::String& category) const
{
    std::vector<int> presetIndices;
    
    for (size_t i = 0; i < m_presets.size(); ++i)
    {
        if (m_presets[i].category == category)
            presetIndices.push_back(static_cast<int>(i));
    }
    
    return presetIndices;
}

std::vector<juce::String> PresetManager::getCategories() const
{
    std::vector<juce::String> categories;
    
    for (const auto& preset : m_presets)
    {
        if (std::find(categories.begin(), categories.end(), preset.category) == categories.end())
            categories.push_back(preset.category);
    }
    
    std::sort(categories.begin(), categories.end());
    return categories;
}

int PresetManager::importPresets(const juce::File& file)
{
    if (!file.exists())
        return 0;
    
    auto xml = juce::XmlDocument::parse(file);
    if (!xml || !xml->hasTagName("ReverbPresets"))
        return 0;
    
    int importedCount = 0;
    
    for (auto* presetXml : xml->getChildIterator())
    {
        if (presetXml->hasTagName("Preset"))
        {
            auto preset = loadPresetFromXml(*presetXml);
            if (isValidPreset(preset))
            {
                preset.isFactoryPreset = false;
                preset.isReadOnly = false;
                m_presets.push_back(preset);
                importedCount++;
            }
        }
    }
    
    if (importedCount > 0)
    {
        sortPresets();
        
        if (m_presetChangeCallback)
            m_presetChangeCallback(m_currentPresetIndex);
    }
    
    return importedCount;
}

bool PresetManager::exportPresets(const juce::File& file, const std::vector<int>& presetIndices)
{
    auto xml = std::make_unique<juce::XmlElement>("ReverbPresets");
    xml->setAttribute("version", "1.0");
    xml->setAttribute("exportDate", juce::Time::getCurrentTime().toString(true, true));
    
    std::vector<int> indicesToExport = presetIndices;
    
    // If no indices specified, export all user presets
    if (indicesToExport.empty())
    {
        for (size_t i = 0; i < m_presets.size(); ++i)
        {
            if (!m_presets[i].isFactoryPreset)
                indicesToExport.push_back(static_cast<int>(i));
        }
    }
    
    // Export specified presets
    for (int index : indicesToExport)
    {
        if (index >= 0 && index < static_cast<int>(m_presets.size()))
        {
            auto presetXml = savePresetToXml(m_presets[static_cast<size_t>(index)]);
            if (presetXml)
                xml->addChildElement(presetXml.release());
        }
    }
    
    // Write to file
    file.getParentDirectory().createDirectory();
    return xml->writeTo(file);
}

juce::File PresetManager::getPresetsDirectory() const
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    return appDataDir.getChildFile("ReverbPlugin").getChildFile("Presets");
}

void PresetManager::refreshPresets()
{
    loadUserPresets();
    sortPresets();
    
    if (m_presetChangeCallback)
        m_presetChangeCallback(m_currentPresetIndex);
}

void PresetManager::setPresetChangeCallback(std::function<void(int)> callback)
{
    m_presetChangeCallback = std::move(callback);
}

void PresetManager::loadFactoryPresets()
{
    auto factoryPresets = FactoryPresets::getAllFactoryPresets();
    
    for (auto& preset : factoryPresets)
    {
        preset.isFactoryPreset = true;
        preset.isReadOnly = true;
        m_presets.push_back(preset);
    }
}

void PresetManager::loadUserPresets()
{
    // Remove existing user presets
    m_presets.erase(
        std::remove_if(m_presets.begin(), m_presets.end(),
            [](const PresetData& preset) { return !preset.isFactoryPreset; }),
        m_presets.end());
    
    auto presetDir = getPresetsDirectory();
    if (!presetDir.exists())
        return;
    
    // Load all .xml files from preset directory
    juce::Array<juce::File> presetFiles;
    presetDir.findChildFiles(presetFiles, juce::File::findFiles, false, "*.xml");
    
    for (const auto& file : presetFiles)
    {
        auto xml = juce::XmlDocument::parse(file);
        if (xml && xml->hasTagName("Preset"))
        {
            auto preset = loadPresetFromXml(*xml);
            if (isValidPreset(preset))
            {
                preset.isFactoryPreset = false;
                preset.isReadOnly = false;
                m_presets.push_back(preset);
            }
        }
    }
}

PresetManager::PresetData PresetManager::loadPresetFromXml(const juce::XmlElement& xml) const
{
    PresetData preset;
    
    preset.name = xml.getStringAttribute("name", "Untitled");
    preset.author = xml.getStringAttribute("author", "");
    preset.description = xml.getStringAttribute("description", "");
    preset.category = xml.getStringAttribute("category", "User");
    preset.dateCreated = juce::Time(xml.getStringAttribute("dateCreated", ""));
    preset.dateModified = juce::Time(xml.getStringAttribute("dateModified", ""));
    
    // Load parameters
    auto parametersXml = xml.getChildByName("Parameters");
    if (parametersXml)
    {
        preset.parameters = juce::ValueTree::fromXml(*parametersXml);
    }
    
    return preset;
}

std::unique_ptr<juce::XmlElement> PresetManager::savePresetToXml(const PresetData& preset) const
{
    auto xml = std::make_unique<juce::XmlElement>("Preset");
    
    xml->setAttribute("name", preset.name);
    xml->setAttribute("author", preset.author);
    xml->setAttribute("description", preset.description);
    xml->setAttribute("category", preset.category);
    xml->setAttribute("dateCreated", preset.dateCreated.toString(true, true));
    xml->setAttribute("dateModified", preset.dateModified.toString(true, true));
    
    // Save parameters
    if (preset.parameters.isValid())
    {
        auto parametersXml = preset.parameters.createXml();
        if (parametersXml)
            xml->addChildElement(parametersXml.release());
    }
    
    return xml;
}

PresetManager::PresetData PresetManager::createPresetFromCurrentState(
    const juce::String& name,
    const juce::String& author,
    const juce::String& description,
    const juce::String& category) const
{
    PresetData preset(name);
    preset.author = author;
    preset.description = description;
    preset.category = category;
    preset.parameters = m_valueTreeState.copyState();
    
    return preset;
}

void PresetManager::applyPresetToValueTree(const PresetData& preset)
{
    if (preset.parameters.isValid())
    {
        // Apply parameters to value tree state
        for (int i = 0; i < preset.parameters.getNumChildren(); ++i)
        {
            auto child = preset.parameters.getChild(i);
            auto paramID = child.getProperty("id").toString();
            auto value = static_cast<float>(child.getProperty("value"));
            
            auto* param = m_valueTreeState.getParameter(paramID);
            if (param)
            {
                param->setValueNotifyingHost(param->convertTo0to1(value));
            }
        }
    }
}

juce::String PresetManager::generateUniquePresetFilename(const juce::String& name) const
{
    // Create a valid filename from the preset name
    auto filename = juce::File::createLegalFileName(name) + ".xml";
    
    auto presetDir = getPresetsDirectory();
    auto file = presetDir.getChildFile(filename);
    
    // If file already exists, add a number suffix
    int counter = 1;
    while (file.exists())
    {
        auto baseName = juce::File::createLegalFileName(name);
        filename = baseName + "_" + juce::String(counter) + ".xml";
        file = presetDir.getChildFile(filename);
        counter++;
    }
    
    return filename;
}

bool PresetManager::presetsMatch(const PresetData& preset1, const PresetData& preset2) const
{
    if (!preset1.parameters.isValid() || !preset2.parameters.isValid())
        return false;
    
    // Compare all parameter values
    for (int i = 0; i < preset1.parameters.getNumChildren(); ++i)
    {
        auto child1 = preset1.parameters.getChild(i);
        auto paramID = child1.getProperty("id").toString();
        auto value1 = static_cast<float>(child1.getProperty("value"));
        
        auto child2 = preset2.parameters.getChildWithProperty("id", paramID);
        if (!child2.isValid())
            return false;
        
        auto value2 = static_cast<float>(child2.getProperty("value"));
        
        // Allow small floating point differences
        if (std::abs(value1 - value2) > 0.001f)
            return false;
    }
    
    return true;
}

void PresetManager::onFileSystemChange()
{
    // Refresh presets when files change in preset directory
    juce::MessageManager::callAsync([this] { refreshPresets(); });
}

void PresetManager::sortPresets()
{
    std::sort(m_presets.begin(), m_presets.end(),
        [](const PresetData& a, const PresetData& b) {
            // Factory presets first, then by category, then by name
            if (a.isFactoryPreset != b.isFactoryPreset)
                return a.isFactoryPreset;
            
            if (a.category != b.category)
                return a.category < b.category;
            
            return a.name < b.name;
        });
}

bool PresetManager::isValidPreset(const PresetData& preset) const
{
    return !preset.name.isEmpty() && preset.parameters.isValid();
}

// FactoryPresets implementation
std::vector<PresetManager::PresetData> FactoryPresets::s_factoryPresets;
bool FactoryPresets::s_initialized = false;

std::vector<PresetManager::PresetData> FactoryPresets::getAllFactoryPresets()
{
    if (!s_initialized)
    {
        initializeFactoryPresets();
        s_initialized = true;
    }
    
    return s_factoryPresets;
}

PresetManager::PresetData FactoryPresets::createFactoryPreset(
    const juce::String& name,
    const juce::String& category,
    const juce::String& description,
    float roomSize,
    float decayTime,
    float preDelay,
    float damping,
    float wetDryMix,
    float earlyLateBalance,
    float diffusion,
    float modulationRate,
    float modulationDepth)
{
    PresetManager::PresetData preset(name);
    preset.category = category;
    preset.description = description;
    preset.author = "Factory";
    preset.isFactoryPreset = true;
    preset.isReadOnly = true;
    
    // Create parameter value tree
    juce::ValueTree parameters("Parameters");
    
    // Add all parameter values
    juce::ValueTree roomSizeTree("Parameter");
    roomSizeTree.setProperty("id", ReverbParams::ROOM_SIZE_ID, nullptr);
    roomSizeTree.setProperty("value", roomSize, nullptr);
    parameters.appendChild(roomSizeTree, nullptr);
    
    juce::ValueTree decayTimeTree("Parameter");
    decayTimeTree.setProperty("id", ReverbParams::DECAY_TIME_ID, nullptr);
    decayTimeTree.setProperty("value", decayTime, nullptr);
    parameters.appendChild(decayTimeTree, nullptr);
    
    juce::ValueTree preDelayTree("Parameter");
    preDelayTree.setProperty("id", ReverbParams::PRE_DELAY_ID, nullptr);
    preDelayTree.setProperty("value", preDelay, nullptr);
    parameters.appendChild(preDelayTree, nullptr);
    
    juce::ValueTree dampingTree("Parameter");
    dampingTree.setProperty("id", ReverbParams::DAMPING_ID, nullptr);
    dampingTree.setProperty("value", damping, nullptr);
    parameters.appendChild(dampingTree, nullptr);
    
    juce::ValueTree wetDryMixTree("Parameter");
    wetDryMixTree.setProperty("id", ReverbParams::WET_DRY_MIX_ID, nullptr);
    wetDryMixTree.setProperty("value", wetDryMix, nullptr);
    parameters.appendChild(wetDryMixTree, nullptr);
    
    juce::ValueTree earlyLateBalanceTree("Parameter");
    earlyLateBalanceTree.setProperty("id", ReverbParams::EARLY_LATE_BALANCE_ID, nullptr);
    earlyLateBalanceTree.setProperty("value", earlyLateBalance, nullptr);
    parameters.appendChild(earlyLateBalanceTree, nullptr);
    
    juce::ValueTree diffusionTree("Parameter");
    diffusionTree.setProperty("id", ReverbParams::DIFFUSION_ID, nullptr);
    diffusionTree.setProperty("value", diffusion, nullptr);
    parameters.appendChild(diffusionTree, nullptr);
    
    juce::ValueTree modulationRateTree("Parameter");
    modulationRateTree.setProperty("id", ReverbParams::MODULATION_RATE_ID, nullptr);
    modulationRateTree.setProperty("value", modulationRate, nullptr);
    parameters.appendChild(modulationRateTree, nullptr);
    
    juce::ValueTree modulationDepthTree("Parameter");
    modulationDepthTree.setProperty("id", ReverbParams::MODULATION_DEPTH_ID, nullptr);
    modulationDepthTree.setProperty("value", modulationDepth, nullptr);
    parameters.appendChild(modulationDepthTree, nullptr);
    
    preset.parameters = parameters;
    
    return preset;
}

void FactoryPresets::initializeFactoryPresets()
{
    s_factoryPresets.clear();
    
    // Small Room
    s_factoryPresets.push_back(createFactoryPreset(
        "Small Room", "Rooms", "Intimate room sound perfect for vocals and acoustic instruments",
        0.3f, 0.8f, 5.0f, 60.0f, 25.0f, 60.0f, 70.0f, 0.8f, 15.0f
    ));
    
    // Medium Hall
    s_factoryPresets.push_back(createFactoryPreset(
        "Medium Hall", "Halls", "Classic concert hall reverb with natural decay",
        1.5f, 2.5f, 25.0f, 40.0f, 35.0f, 50.0f, 80.0f, 1.2f, 20.0f
    ));
    
    // Large Cathedral
    s_factoryPresets.push_back(createFactoryPreset(
        "Large Cathedral", "Churches", "Spacious cathedral with long, smooth decay",
        3.5f, 6.0f, 80.0f, 20.0f, 40.0f, 40.0f, 85.0f, 0.6f, 30.0f
    ));
    
    // Plate Reverb
    s_factoryPresets.push_back(createFactoryPreset(
        "Plate Reverb", "Vintage", "Classic plate reverb sound with bright character",
        0.8f, 3.2f, 2.0f, 75.0f, 45.0f, 70.0f, 60.0f, 2.1f, 35.0f
    ));
    
    // Spring Reverb
    s_factoryPresets.push_back(createFactoryPreset(
        "Spring Reverb", "Vintage", "Vintage spring reverb with characteristic boing",
        0.4f, 1.5f, 8.0f, 85.0f, 50.0f, 80.0f, 45.0f, 3.5f, 45.0f
    ));
    
    // Ambient Space
    s_factoryPresets.push_back(createFactoryPreset(
        "Ambient Space", "Ambient", "Ethereal ambient reverb for soundscapes",
        4.5f, 8.0f, 150.0f, 10.0f, 60.0f, 30.0f, 90.0f, 0.4f, 50.0f
    ));
    
    // Vocal Reverb
    s_factoryPresets.push_back(createFactoryPreset(
        "Vocal Reverb", "Vocal", "Tailored for vocal processing with clarity",
        1.2f, 1.8f, 40.0f, 55.0f, 30.0f, 65.0f, 75.0f, 1.0f, 20.0f
    ));
    
    // Drum Reverb
    s_factoryPresets.push_back(createFactoryPreset(
        "Drum Reverb", "Drums", "Punchy reverb designed for drum processing",
        0.6f, 1.2f, 15.0f, 70.0f, 35.0f, 75.0f, 65.0f, 1.8f, 25.0f
    ));
}