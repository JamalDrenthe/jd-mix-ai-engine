#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "gui/theme/JDMixLookAndFeel.h"
#include "gui/views/View1EqCleanDecay.h"
#include "gui/views/View2AutoTuneMatrix.h"
#include "gui/views/View3ChromaticOrb.h"
#include "gui/views/View4SpatialSends.h"
#include "gui/views/View5StemMasterRadar.h"
#include "core/UpdateChecker.h"

class JDMixAIEngineAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit JDMixAIEngineAudioProcessorEditor(JDMixAIEngineAudioProcessor&);
    ~JDMixAIEngineAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override; // 60-120 Hz lock-free UI telemetry refresh

    JDMixAIEngineAudioProcessor& processor;
    jdmix::gui::JDMixLookAndFeel lookAndFeel;

    // Header Controls
    juce::ComboBox trackRoleSelector;
    juce::TextButton viewTabs[5];
    int activeViewIndex { 0 };

    juce::TextButton aSlotButton { "A" };
    juce::TextButton bSlotButton { "B" };
    juce::TextButton deltaButton { "Δ" };
    juce::TextButton updatePillButton { "🚀 Update Available" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> trackRoleAttach;

    // 5 Workspaces
    jdmix::gui::View1EqCleanDecay view1;
    jdmix::gui::View2AutoTuneMatrix view2;
    jdmix::gui::View3ChromaticOrb view3;
    jdmix::gui::View4SpatialSends view4;
    jdmix::gui::View5StemMasterRadar view5;

    jdmix::core::UpdateChecker updateChecker { "1.0.0" };

    void updateActiveView(int newIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JDMixAIEngineAudioProcessorEditor)
};
