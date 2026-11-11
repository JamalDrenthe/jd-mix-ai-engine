#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../theme/JDMixLookAndFeel.h"
#include "../../ai/GeminiClient.h"

namespace jdmix::gui
{
    class View3ChromaticOrb : public juce::Component
    {
    public:
        View3ChromaticOrb(juce::AudioProcessorValueTreeState& apvts, ai::GeminiClient& aiClient);
        ~View3ChromaticOrb() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void setAudioTelemetry(float rms, float formantShift);
        std::function<void()> onExecuteAiPlan;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseMove(const juce::MouseEvent& e) override;
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    private:
        juce::AudioProcessorValueTreeState& apvts;
        ai::GeminiClient& aiClient;

        juce::ComboBox modelSelector;
        juce::TextButton executeAiButton { "✨ Execute AI Table Plan" };
        juce::ToggleButton rangeMatchingToggle { "Range Matching" };

        float currentRms { 0.0f };
        float currentFormant { 0.0f };
        float animationPhase { 0.0f };

        // 3D Motion Design & Shockwave State
        juce::Point<float> mousePos;
        juce::Point<float> shockwavePos;
        float shockwaveRadius { 0.0f };
        float shockwaveAlpha { 0.0f };
        float timelineZoom { 1.0f };
        float timelineOffset { 0.0f };
        juce::Rectangle<float> cachedOrbArea;
        juce::Rectangle<float> cachedTimelineArea;

        void drawLiquidOrb(juce::Graphics& g, juce::Rectangle<float> orbArea);
        void drawWaveformTimeline(juce::Graphics& g, juce::Rectangle<float> timelineArea);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(View3ChromaticOrb)
    };
}
