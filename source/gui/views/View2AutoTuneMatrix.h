#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../theme/JDMixLookAndFeel.h"

namespace jdmix::gui
{
    class View2AutoTuneMatrix : public juce::Component
    {
    public:
        explicit View2AutoTuneMatrix(juce::AudioProcessorValueTreeState& apvts);
        ~View2AutoTuneMatrix() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void updatePitchData(const juce::String& noteName, float centDeviation);

    private:
        juce::AudioProcessorValueTreeState& apvts;

        // Left Column Controls
        juce::ComboBox keySelector;
        juce::ComboBox scaleSelector;
        juce::ComboBox modeSelector;
        juce::TextButton pillNotes[12];
        bool noteActive[12] { true, true, true, true, true, true, true, true, true, true, true, true };

        // Right Column Dials
        juce::Slider retuneSpeedSlider;
        juce::Slider humanizeSlider;
        juce::Slider flexTuneSlider;
        juce::ToggleButton modernClassicToggle { "Modern / Classic" };

        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retuneAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> humanizeAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flexTuneAttach;

        juce::String currentNote { "F#" };
        float currentCent { 0.0f };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(View2AutoTuneMatrix)
    };
}
