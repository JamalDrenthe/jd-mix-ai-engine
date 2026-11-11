#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include "../theme/JDMixLookAndFeel.h"
#include "../../ipc/LocalIpcBus.h"

namespace jdmix::gui
{
    class View5StemMasterRadar : public juce::Component
    {
    public:
        View5StemMasterRadar(juce::AudioProcessorValueTreeState& apvts, ipc::LocalIpcBus& ipc);
        ~View5StemMasterRadar() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

        void updateRadarData();

    private:
        juce::AudioProcessorValueTreeState& apvts;
        ipc::LocalIpcBus& ipcBus;

        // Top Horizontal Sliders
        juce::Slider targetLufsSlider;
        juce::Slider vocalPocketSlider;
        juce::Slider monoBassSlider;

        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> targetLufsAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vocalPocketAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> monoBassAttach;

        // 40-Slot Memory Matrix
        juce::TextButton memoryButtons[40];
        int activeMemorySlot { 0 };

        // 48 Sectors Polar Radar Data
        std::array<float, 48> sectorEnergiesDb {};

        void drawPolarRadar(juce::Graphics& g, juce::Rectangle<float> area);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(View5StemMasterRadar)
    };
}
