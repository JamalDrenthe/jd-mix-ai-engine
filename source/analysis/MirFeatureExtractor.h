#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <string>
#include "LufsMeter.h"
#include "SpectrumAnalyzer.h"

namespace jdmix::analysis
{
    struct MirAcousticSnapshot
    {
        float lufsIntegrated { -14.0f };
        float lufsShortTerm  { -14.0f };
        float truePeakDb     { -1.0f };
        float crestFactorPsr { 8.0f };

        float spectralCentroidHz { 2500.0f };
        float spectralTiltDb     { -4.5f };
        float sibilanceRatio     { 0.12f };
        float mudEnergyRatio     { 0.18f };

        std::string detectedKey   { "F#" };
        std::string detectedScale { "Minor" };
        std::string suggestedRole { "Lead Vocal" };

        std::array<float, SpectrumAnalyzer::numBands32> spectrum32 {};

        // JSON serialization helper for Gemini API payload
        std::string toJsonString() const;
    };

    class MirFeatureExtractor
    {
    public:
        MirFeatureExtractor() = default;

        void prepare(double sampleRate);
        void process(const juce::AudioBuffer<float>& buffer,
                     const LufsMeter& lufs,
                     const SpectrumAnalyzer& spectrum);

        MirAcousticSnapshot getLatestSnapshot() const;

    private:
        double currentSampleRate { 44100.0 };
        MirAcousticSnapshot snapshot;

        // Auto-classifier heuristics
        void updateClassification();
    };
}
