#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include "../theme/JDMixLookAndFeel.h"
#include "../../analysis/SpectrumAnalyzer.h"

namespace jdmix::gui
{
    class View1EqCleanDecay : public juce::Component
    {
    public:
        explicit View1EqCleanDecay(juce::AudioProcessorValueTreeState& apvts);
        ~View1EqCleanDecay() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void updateSpectrum(const std::array<float, analysis::SpectrumAnalyzer::numBands64>& spectrum,
                            const std::array<float, analysis::SpectrumAnalyzer::numBands64>* maskingSpectrum = nullptr);

    private:
        juce::AudioProcessorValueTreeState& apvts;

        // Top Macro Ribbon (6 sliders + Central Hero Arch Dial)
        juce::Slider brightnessSlider;
        juce::Slider characterSlider;
        juce::Slider distanceSlider;
        juce::Slider decayRateSlider;
        juce::Slider stereoWidthSlider;
        juce::Slider mixSlider;
        juce::Slider heroArchDial; // Space / Target Gain

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> brightnessAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> characterAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distanceAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayRateAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoWidthAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> heroArchAttach;

        // Cached spectrum curves for 120 FPS paint
        std::array<float, analysis::SpectrumAnalyzer::numBands64> currentSpectrum {};
        std::array<float, analysis::SpectrumAnalyzer::numBands64> currentGhostTrail {};
        std::array<float, analysis::SpectrumAnalyzer::numBands64> maskSpectrum {};
        bool hasMasking { false };

        void drawSpectrumGrid(juce::Graphics& g, juce::Rectangle<float> area);
        void drawSmokeFft(juce::Graphics& g, juce::Rectangle<float> area);
        void drawCyanDynamicCurve(juce::Graphics& g, juce::Rectangle<float> area);
        void drawGoldEqCurve(juce::Graphics& g, juce::Rectangle<float> area);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(View1EqCleanDecay)
    };
}
