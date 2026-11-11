cmake_minimum_required(VERSION 3.24)
project(JDMixAIEngine VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# JUCE 8 Setup
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
endif()

# Main VST3 Plugin Target
juce_add_plugin(JDMixAIEngine
    COMPANY_NAME "JD Audio Labs"
    BUNDLE_ID "com.jdaudiolabs.jdmixaieengine"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE JdAl
    PLUGIN_CODE JdMx
    FORMATS VST3 Standalone
    PRODUCT_NAME "JD Mix AI Engine"
    VST3_CATEGORIES "Fx" "Dynamics" "EQ" "Mastering"
)

target_sources(JDMixAIEngine PRIVATE
    source/PluginProcessor.cpp
    source/PluginProcessor.h
    source/PluginEditor.cpp
    source/PluginEditor.h

    source/core/VariableBlockFifo.h
    source/core/LatencyManager.h
    source/core/SafetyGuard.h
    source/core/ParamRegistry.h

    source/analysis/LufsMeter.cpp
    source/analysis/LufsMeter.h
    source/analysis/SpectrumAnalyzer.cpp
    source/analysis/SpectrumAnalyzer.h
    source/analysis/MirFeatureExtractor.cpp
    source/analysis/MirFeatureExtractor.h

    source/ipc/LocalIpcBus.cpp
    source/ipc/LocalIpcBus.h

    source/ai/GeminiSchema.h
    source/ai/GeminiClient.cpp
    source/ai/GeminiClient.h

    source/dsp/Stage00_Restoration.h
    source/dsp/Stage01_ClipGainPreRider.h
    source/dsp/Stage02_SubtractiveDynEq.h
    source/dsp/Stage03_DeEsserPass1.h
    source/dsp/Stage04_SerialParallelComp.h
    source/dsp/Stage05_DeEsserPass2.h
    source/dsp/Stage06_PitchStudio.h
    source/dsp/Stage07_HarmonicSaturator.h
    source/dsp/Stage08_AdditiveToneEq.h
    source/dsp/Stage09_SpatialSends.h
    source/dsp/Stage10_11_StemMasterLimiter.h

    source/gui/theme/JDMixLookAndFeel.h
    source/gui/views/View1EqCleanDecay.cpp
    source/gui/views/View1EqCleanDecay.h
    source/gui/views/View2AutoTuneMatrix.cpp
    source/gui/views/View2AutoTuneMatrix.h
    source/gui/views/View3ChromaticOrb.cpp
    source/gui/views/View3ChromaticOrb.h
    source/gui/views/View4SpatialSends.cpp
    source/gui/views/View4SpatialSends.h
    source/gui/views/View5StemMasterRadar.cpp
    source/gui/views/View5StemMasterRadar.h
)

target_compile_definitions(JDMixAIEngine PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JDM_FL_STUDIO_OPTIMIZED=1
)

target_link_libraries(JDMixAIEngine PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

find_package(CURL)
if(CURL_FOUND)
    target_link_libraries(JDMixAIEngine PRIVATE CURL::libcurl)
endif()

if(WIN32)
    target_link_libraries(JDMixAIEngine PRIVATE
        juce::juce_audio_devices
        juce::juce_audio_utils
    )
endif()

# Test executables
find_package(CURL)
add_executable(FifoFuzzTest tests/FifoFuzzTest.cpp)
target_link_libraries(FifoFuzzTest PRIVATE juce::juce_audio_basics)
if(CURL_FOUND)
    target_link_libraries(FifoFuzzTest PRIVATE CURL::libcurl)
endif()

add_executable(DspChainTest tests/DspChainTest.cpp)
target_link_libraries(DspChainTest PRIVATE juce::juce_audio_basics juce::juce_dsp)
if(CURL_FOUND)
    target_link_libraries(DspChainTest PRIVATE CURL::libcurl)
endif()
