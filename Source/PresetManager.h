#pragma once

#include <JuceHeader.h>
#include "Parameters.h"

/**
 * @brief Manages plugin presets including factory and user presets
 * 
 * This class handles saving, loading, and organizing presets for the
 * reverb plugin. It supports both factory presets and user-created presets.
 */
class PresetManager
{
public:
    struct PresetData
    {
        juce::String name;
        juce::String author;
        juce::String description;
        juce::String category;
        juce::ValueTree parameters;
        juce::Time dateCreated;
        juce::Time dateModified;
        bool isFactoryPreset;
        bool isReadOnly;
        
        PresetData();
        PresetData(const juce::String& presetName);
    };
    
    explicit PresetManager(juce::AudioProcessorValueTreeState& valueTreeState);
    ~PresetManager();
    
    /**
     * @brief Initialize the preset manager
     * Loads factory presets and scans for user presets
     */
    void initialize();
    
    /**
     * @brief Get the number of available presets
     * @return Total number of presets (factory + user)
     */
    int getNumPresets() const;
    
    /**
     * @brief Get preset data by index
     * @param index Preset index
     * @return Preset data structure
     */
    PresetData getPreset(int index) const;
    
    /**
     * @brief Get preset name by index
     * @param index Preset index
     * @return Preset name
     */
    juce::String getPresetName(int index) const;
    
    /**
     * @brief Get current preset index
     * @return Index of currently loaded preset (-1 if none)
     */
    int getCurrentPresetIndex() const { return m_currentPresetIndex; }
    
    /**
     * @brief Load a preset by index
     * @param index Preset index to load
     * @return True if preset was loaded successfully
     */
    bool loadPreset(int index);
    
    /**
     * @brief Load a preset by name
     * @param name Preset name to load
     * @return True if preset was loaded successfully
     */
    bool loadPreset(const juce::String& name);
    
    /**
     * @brief Save current state as a new preset
     * @param name Name for the new preset
     * @param author Author name (optional)
     * @param description Description (optional)
     * @param category Category (optional)
     * @return True if preset was saved successfully
     */
    bool savePreset(const juce::String& name, 
                   const juce::String& author = "",
                   const juce::String& description = "",
                   const juce::String& category = "User");
    
    /**
     * @brief Delete a user preset
     * @param index Preset index to delete
     * @return True if preset was deleted successfully
     */
    bool deletePreset(int index);
    
    /**
     * @brief Rename a preset
     * @param index Preset index
     * @param newName New name for the preset
     * @return True if preset was renamed successfully
     */
    bool renamePreset(int index, const juce::String& newName);
    
    /**
     * @brief Check if current settings match any preset
     * @return Index of matching preset (-1 if no match)
     */
    int findMatchingPreset() const;
    
    /**
     * @brief Get all presets in a specific category
     * @param category Category name
     * @return Vector of preset indices in the category
     */
    std::vector<int> getPresetsInCategory(const juce::String& category) const;
    
    /**
     * @brief Get all available categories
     * @return Vector of category names
     */
    std::vector<juce::String> getCategories() const;
    
    /**
     * @brief Import presets from a file
     * @param file File to import presets from
     * @return Number of presets imported
     */
    int importPresets(const juce::File& file);
    
    /**
     * @brief Export presets to a file
     * @param file File to export presets to
     * @param presetIndices Indices of presets to export (empty = all user presets)
     * @return True if export was successful
     */
    bool exportPresets(const juce::File& file, const std::vector<int>& presetIndices = {});
    
    /**
     * @brief Get the default presets directory
     * @return Directory where user presets are stored
     */
    juce::File getPresetsDirectory() const;
    
    /**
     * @brief Refresh preset list (scan for new files)
     */
    void refreshPresets();
    
    /**
     * @brief Set callback for preset changes
     * @param callback Function to call when presets change
     */
    void setPresetChangeCallback(std::function<void(int)> callback);
    
private:
    juce::AudioProcessorValueTreeState& m_valueTreeState;
    std::vector<PresetData> m_presets;
    int m_currentPresetIndex;
    std::function<void(int)> m_presetChangeCallback;
    
    // File watching for preset directory changes
    std::unique_ptr<juce::FileSystemWatcher> m_fileWatcher;
    
    /**
     * @brief Load factory presets from embedded data
     */
    void loadFactoryPresets();
    
    /**
     * @brief Load user presets from files
     */
    void loadUserPresets();
    
    /**
     * @brief Load a preset from XML
     * @param xml XML element containing preset data
     * @return Loaded preset data
     */
    PresetData loadPresetFromXml(const juce::XmlElement& xml) const;
    
    /**
     * @brief Save a preset to XML
     * @param preset Preset data to save
     * @return XML element containing preset data
     */
    std::unique_ptr<juce::XmlElement> savePresetToXml(const PresetData& preset) const;
    
    /**
     * @brief Create a preset from current parameter values
     * @param name Preset name
     * @param author Author name
     * @param description Description
     * @param category Category
     * @return Created preset data
     */
    PresetData createPresetFromCurrentState(const juce::String& name,
                                          const juce::String& author,
                                          const juce::String& description,
                                          const juce::String& category) const;
    
    /**
     * @brief Apply preset parameters to the value tree state
     * @param preset Preset to apply
     */
    void applyPresetToValueTree(const PresetData& preset);
    
    /**
     * @brief Generate a unique filename for a preset
     * @param name Preset name
     * @return Unique filename
     */
    juce::String generateUniquePresetFilename(const juce::String& name) const;
    
    /**
     * @brief Check if two presets have the same parameters
     * @param preset1 First preset
     * @param preset2 Second preset
     * @return True if parameters match
     */
    bool presetsMatch(const PresetData& preset1, const PresetData& preset2) const;
    
    /**
     * @brief Handle file system changes in preset directory
     */
    void onFileSystemChange();
    
    /**
     * @brief Sort presets by category and name
     */
    void sortPresets();
    
    /**
     * @brief Validate preset data
     * @param preset Preset to validate
     * @return True if preset is valid
     */
    bool isValidPreset(const PresetData& preset) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};

/**
 * @brief Factory preset definitions
 * 
 * Contains all factory presets embedded in the plugin.
 */
class FactoryPresets
{
public:
    /**
     * @brief Get all factory preset definitions
     * @return Vector of factory presets
     */
    static std::vector<PresetManager::PresetData> getAllFactoryPresets();
    
    /**
     * @brief Create a factory preset with given parameters
     * @param name Preset name
     * @param category Preset category
     * @param description Preset description
     * @param roomSize Room size parameter
     * @param decayTime Decay time parameter
     * @param preDelay Pre-delay parameter
     * @param damping Damping parameter
     * @param wetDryMix Wet/dry mix parameter
     * @param earlyLateBalance Early/late balance parameter
     * @param diffusion Diffusion parameter
     * @param modulationRate Modulation rate parameter
     * @param modulationDepth Modulation depth parameter
     * @return Factory preset data
     */
    static PresetManager::PresetData createFactoryPreset(
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
        float modulationDepth
    );
    
private:
    /**
     * @brief Initialize all factory presets
     */
    static void initializeFactoryPresets();
    
    static std::vector<PresetManager::PresetData> s_factoryPresets;
    static bool s_initialized;
};