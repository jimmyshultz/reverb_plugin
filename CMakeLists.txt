cmake_minimum_required(VERSION 3.15)

project(ReverbPlugin VERSION 1.0.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectory for JUCE if using as subdirectory
# find_package(juce REQUIRED) # or add_subdirectory(JUCE)

# Plugin formats to build
set(FORMATS AU VST3 Standalone)

# Add the plugin target
juce_add_plugin(ReverbPlugin
    COMPANY_NAME "Your Company"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Rvbp
    FORMATS ${FORMATS}
    PRODUCT_NAME "Reverb Plugin"
    DESCRIPTION "High-quality algorithmic reverb"
    VST3_CATEGORIES "Fx" "Reverb"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

# Add source files
target_sources(ReverbPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ReverbEngine.cpp
    Source/Parameters.cpp
    Source/PresetManager.cpp
    Source/DSP/DelayLine.cpp
    Source/DSP/AllpassFilter.cpp
    Source/DSP/CombFilter.cpp
    Source/DSP/LowpassFilter.cpp
    Source/DSP/Modulator.cpp
)

# Preprocessor definitions
target_compile_definitions(ReverbPlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link JUCE libraries
target_link_libraries(ReverbPlugin PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(ReverbPlugin PRIVATE JUCE_WINDOWS=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(ReverbPlugin PRIVATE JUCE_MAC=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(ReverbPlugin PRIVATE JUCE_LINUX=1)
endif()

# Copy resources
add_custom_command(TARGET ReverbPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Resources
    $<TARGET_FILE_DIR:ReverbPlugin>/Resources
)

# Install targets
install(TARGETS ReverbPlugin
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Package configuration
set(CPACK_PACKAGE_NAME "ReverbPlugin")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "High-quality algorithmic reverb plugin")
set(CPACK_PACKAGE_CONTACT "your.email@company.com")

include(CPack)