#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <atomic>

namespace jdmix::analysis
{
    /**
     * LufsMeter
     * Complies with ITU-R BS.1770-4 standard for loudness measurement.
     * Computes Momentary, Short-Term, Integrated LUFS, and 4x True-Peak (dBTP).
     */
    class LufsMeter
    {
    public:
        LufsMeter() = default;

        void prepare(double sampleRate, int maxBlockSize);
        void reset();
        void process(const juce::AudioBuffer<float>& buffer);

        float getMomentaryLufs() const noexcept { return momentaryLufs.load(std::memory_order_relaxed); }
        float getShortTermLufs() const noexcept { return shortTermLufs.load(std::memory_order_relaxed); }
        float getIntegratedLufs() const noexcept { return integratedLufs.load(std::memory_order_relaxed); }
        float getMaxTruePeak() const noexcept { return maxTruePeak.load(std::memory_order_relaxed); }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::IIR::Filter<float> kFilterStage1[2]; // Pre-filter high-shelf
        juce::dsp::IIR::Filter<float> kFilterStage2[2]; // RLB weighting high-pass

        // Ring buffers for 400 ms (Momentary) and 3000 ms (Short-Term)
        std::vector<float> momentaryBuffer;
        std::vector<float> shortTermBuffer;
        int momentaryIndex { 0 };
        int shortTermIndex { 0 };
        int momentaryLength { 0 };
        int shortTermLength { 0 };

        // Integrated loudness gating accumulation
        std::vector<float> blockPowers;
        double totalGatedPower { 0.0 };
        int gatedBlockCount { 0 };

        std::atomic<float> momentaryLufs { -100.0f };
        std::atomic<float> shortTermLufs { -100.0f };
        std::atomic<float> integratedLufs { -100.0f };
        std::atomic<float> maxTruePeak { -100.0f };

        // 4x True Peak oversampler
        juce::dsp::Oversampling<float> oversampler { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true };
    };
}
