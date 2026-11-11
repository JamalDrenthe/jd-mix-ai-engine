#include "SpectrumAnalyzer.h"
#include <cmath>

namespace jdmix::analysis
{
    SpectrumAnalyzer::SpectrumAnalyzer()
        : forwardFFT(fftOrder),
          window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        fifo.fill(0.0f);
        fftData.fill(0.0f);
        bands64.fill(-100.0f);
        bands32.fill(-100.0f);
        calculateBinBoundaries();
    }

    void SpectrumAnalyzer::prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        calculateBinBoundaries();
    }

    void SpectrumAnalyzer::calculateBinBoundaries()
    {
        // 64 log bands between 20 Hz and 20000 Hz
        for (int i = 0; i <= numBands64; ++i)
        {
            float freq = 20.0f * std::pow(1000.0f, static_cast<float>(i) / numBands64);
            int bin = static_cast<int>(freq * fftSize / currentSampleRate);
            binBoundaries64[i] = juce::jlimit(0, fftSize / 2, bin);
        }

        // 32 1/3-octave bands
        for (int i = 0; i <= numBands32; ++i)
        {
            float freq = 20.0f * std::pow(1000.0f, static_cast<float>(i) / numBands32);
            int bin = static_cast<int>(freq * fftSize / currentSampleRate);
            binBoundaries32[i] = juce::jlimit(0, fftSize / 2, bin);
        }
    }

    void SpectrumAnalyzer::pushAudio(const juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        if (numSamples == 0 || numChannels == 0)
            return;

        for (int s = 0; s < numSamples; ++s)
        {
            float mono = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                mono += buffer.getSample(ch, s);
            mono /= numChannels;

            if (fifoIndex < fftSize)
            {
                fifo[fifoIndex++] = mono;
            }

            if (fifoIndex >= fftSize)
            {
                if (!nextFFTBlockReady.load(std::memory_order_relaxed))
                {
                    std::copy(fifo.begin(), fifo.begin() + fftSize, fftData.begin());
                    std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);
                    nextFFTBlockReady.store(true, std::memory_order_release);
                }
                fifoIndex = 0;
            }
        }
    }

    void SpectrumAnalyzer::updateSpectrum()
    {
        if (nextFFTBlockReady.load(std::memory_order_acquire))
        {
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

            // 64-band calculation with smooth decay
            for (int b = 0; b < numBands64; ++b)
            {
                int startBin = binBoundaries64[b];
                int endBin   = juce::jmax(startBin + 1, binBoundaries64[b + 1]);

                float sum = 0.0f;
                for (int bin = startBin; bin < endBin; ++bin)
                    sum += fftData[bin];
                float avg = sum / (endBin - startBin);
                float db = juce::Decibels::gainToDecibels(avg / fftSize, -100.0f);

                // Smooth decay
                if (db > bands64[b])
                    bands64[b] = db;
                else
                    bands64[b] = bands64[b] * 0.88f + db * 0.12f;
            }

            // 32-band calculation
            for (int b = 0; b < numBands32; ++b)
            {
                int startBin = binBoundaries32[b];
                int endBin   = juce::jmax(startBin + 1, binBoundaries32[b + 1]);

                float sum = 0.0f;
                for (int bin = startBin; bin < endBin; ++bin)
                    sum += fftData[bin];
                float avg = sum / (endBin - startBin);
                float db = juce::Decibels::gainToDecibels(avg / fftSize, -100.0f);

                if (db > bands32[b])
                    bands32[b] = db;
                else
                    bands32[b] = bands32[b] * 0.88f + db * 0.12f;
            }

            nextFFTBlockReady.store(false, std::memory_order_release);
        }
    }

    void SpectrumAnalyzer::get64Bands(std::array<float, numBands64>& dest) const noexcept
    {
        dest = bands64;
    }

    void SpectrumAnalyzer::get32Bands(std::array<float, numBands32>& dest) const noexcept
    {
        dest = bands32;
    }
}
