cmake_minimum_required(VERSION 3.22)
if(CMAKE_HOST_APPLE AND NOT DEFINED CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures")
endif()
project(CopycatDiffuse VERSION 1.0.1)

# ============================================================================
# JUCE Path Configuration (supports local dev and CI builds)
# ============================================================================
if(NOT DEFINED JUCE_DIR)
    if(DEFINED ENV{JUCE_DIR})
        set(JUCE_DIR "$ENV{JUCE_DIR}")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
        set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    elseif(WIN32)
        set(JUCE_DIR "C:/JUCE")
    else()
        set(JUCE_DIR "/Applications/JUCE")
    endif()
endif()

message(STATUS "Using JUCE from: ${JUCE_DIR}")
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE source not found. Set -DJUCE_DIR=/path/to/JUCE (JUCE 8.0.12).")
endif()
add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

option(COPYCAT_COPY_PLUGIN_AFTER_BUILD "Install plugins after building" OFF)
set(COPYCAT_PLUGIN_FORMATS VST3)
if(APPLE)
    list(APPEND COPYCAT_PLUGIN_FORMATS AU)
endif()

# ============================================================================
# Binary Resources (Fonts)
# ============================================================================
juce_add_binary_data(FontResources
    SOURCES
        resources/fonts/Montserrat-Bold.ttf
        resources/fonts/Montserrat-Medium.ttf)

# ============================================================================
# Plugin Target
# ============================================================================
juce_add_plugin(CopycatDiffuse
    COMPANY_NAME "CopyCat"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${COPYCAT_COPY_PLUGIN_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE "CpCt"
    PLUGIN_CODE "CCDF"
    FORMATS ${COPYCAT_PLUGIN_FORMATS}
    BUNDLE_ID "com.copycat.diffuse"
    PRODUCT_NAME "CopyCat Diffuse")

# ============================================================================
# Source Files
# ============================================================================
target_sources(CopycatDiffuse
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginProcessor.h
        src/PluginEditor.cpp
        src/PluginEditor.h
        
        src/dsp/STFTProcessor.cpp
        src/dsp/STFTProcessor.h
        src/dsp/DiffuseEngine.cpp
        src/dsp/DiffuseEngine.h
        
        src/ui/CustomLookAndFeel.cpp
        src/ui/CustomLookAndFeel.h
        src/ui/UIConstants.h
        src/ui/ShadowButton.h)

# ============================================================================
# Dependencies
# ============================================================================
target_link_libraries(CopycatDiffuse
    PRIVATE
        FontResources
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_compile_features(CopycatDiffuse PRIVATE cxx_std_17)

# ============================================================================
# Compile Definitions
# ============================================================================
target_compile_definitions(CopycatDiffuse
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
