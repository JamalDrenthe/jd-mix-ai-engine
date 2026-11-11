#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>

namespace jdmix::analysis
{
    class SpectrumAnalyzer
    {
    public:
        static constexpr int fftOrder = 11; // 2048 points
        static constexpr int fftSize  = 1 << fftOrder;
        static constexpr int numBands64 = 64;
        static constexpr int numBands32 = 32;

        SpectrumAnalyzer();

        void prepare(double sampleRate);
        void pushAudio(const juce::AudioBuffer<float>& buffer);
        void updateSpectrum();

        // 64-band envelope for GUI smoke FFT and Local IPC (values in dB: -100 to 0)
        void get64Bands(std::array<float, numBands64>& dest) const noexcept;

        // 32-band 1/3-octave envelope for Gemini MIR payload (values in dB: -100 to 0)
        void get32Bands(std::array<float, numBands32>& dest) const noexcept;

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::FFT forwardFFT;
        juce::dsp::WindowingFunction<float> window;

        std::array<float, fftSize * 2> fifo;
        std::array<float, fftSize * 2> fftData;
        int fifoIndex { 0 };
        std::atomic<bool> nextFFTBlockReady { false };

        std::array<float, numBands64> bands64 {};
        std::array<float, numBands32> bands32 {};

        std::array<int, numBands64 + 1> binBoundaries64 {};
        std::array<int, numBands32 + 1> binBoundaries32 {};

        void calculateBinBoundaries();
    };
}
