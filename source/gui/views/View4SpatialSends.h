#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../theme/JDMixLookAndFeel.h"

namespace jdmix::gui
{
    class View4SpatialSends : public juce::Component
    {
    public:
        explicit View4SpatialSends(juce::AudioProcessorValueTreeState& apvts);
        ~View4SpatialSends() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseMove(const juce::MouseEvent& e) override;
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    private:
        juce::AudioProcessorValueTreeState& apvts;

        // Left Mod Strips
        juce::Slider preDelaySlider;
        juce::Slider slapTimeSlider;
        juce::Slider plateSendSlider;
        juce::Slider slapSendSlider;

        // Hardware Mixer Bay Dials
        juce::Slider mixInputSlider;
        juce::Slider mixPanner1Slider;
        juce::Slider mixPanner2Slider;
        juce::Slider mixOutputSlider;
        juce::Slider mixBlendSlider;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> slapTimeAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> plateSendAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> slapSendAttach;

        // 3D Motion Design State
        float spherePitch { 0.25f }; // Radians
        float sphereYaw { 0.0f };
        float sphereZoom { 1.0f };
        juce::Point<float> lastMousePos;

        // Click Shockwave Ripple
        juce::Point<float> clickRipplePos;
        float clickRippleRadius { 0.0f };
        float clickRippleAlpha { 0.0f };

        // Parallax Card Tilt
        float cardTiltX { 0.0f };
        float cardTiltY { 0.0f };
        juce::Rectangle<float> cachedGlobeArea;

        void draw3dWireframeSphere(juce::Graphics& g, juce::Rectangle<float> area);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(View4SpatialSends)
    };
}
