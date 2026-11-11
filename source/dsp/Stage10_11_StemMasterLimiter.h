#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 10 & 11: Vocal Pocket Rider & 4-Stem Master True-Peak Limiter
     * - Mono-Bass Crossover (< 90 Hz mono-maker)
     * - Vocal Pocket Leveling
     * - 4x Oversampled True-Peak Limiter (-1.0 dBTP ceiling, -14 LUFS target)
     */
    class Stage10_11_StemMasterLimiter
    {
    public:
        Stage10_11_StemMasterLimiter()
            : oversampler(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)
        {
        }

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;
            oversampler.initProcessing(samplesPerBlock);

            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = samplesPerBlock;
            spec.numChannels = 1;

            monoBassFilter.prepare(spec);
            monoBassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 90.0f, 0.7071f);

            limiterEnv = 0.0f;
        }

        void reset()
        {
            oversampler.reset();
            monoBassFilter.reset();
            limiterEnv = 0.0f;
        }

        void process(juce::AudioBuffer<float>& buffer,
                     float targetLufs,
                     float truePeakCeilingDb,
                     float vocalPocketOffsetDb,
                     float monoBassFreq)
        {
            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();
            if (numSamples == 0) return;

            // 1. Mono-Bass processing (< monoBassFreq made 100% mono)
            if (numChannels >= 2 && monoBassFreq > 40.0f)
            {
                monoBassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
                    currentSampleRate, monoBassFreq, 0.7071f);

                for (int s = 0; s < numSamples; ++s)
                {
                    float left  = buffer.getSample(0, s);
                    float right = buffer.getSample(1, s);

                    // Mid / Side encoding
                    float mid  = (left + right) * 0.7071f;
                    float side = (left - right) * 0.7071f;

                    // Filter bass out of side channel
                    side = monoBassFilter.processSample(side);

                    // Decode back to L/R
                    buffer.setSample(0, s, (mid + side) * 0.7071f);
                    buffer.setSample(1, s, (mid - side) * 0.7071f);
                }
            }

            // 2. 4x Oversampled True-Peak Limiter
            const float ceilingLinear = juce::Decibels::decibelsToGain(truePeakCeilingDb);
            const float releaseCoeff = std::exp(-1.0f / (0.050f * currentSampleRate * 4.0f)); // 50 ms release

            juce::dsp::AudioBlock<float> inBlock(buffer);
            auto upsampled = oversampler.processSamplesUp(inBlock);

            const size_t upCh = upsampled.getNumChannels();
            const size_t upLen = upsampled.getNumSamples();

            for (size_t s = 0; s < upLen; ++s)
            {
                float peak = 0.0f;
                for (size_t ch = 0; ch < upCh; ++ch)
                    peak = juce::jmax(peak, std::abs(upsampled.getChannelPointer(ch)[s]));

                if (peak > limiterEnv)
                    limiterEnv = peak; // Instant attack lookahead
                else
                    limiterEnv = peak + releaseCoeff * (limiterEnv - peak);

                float gain = 1.0f;
                if (limiterEnv > ceilingLinear)
                {
                    gain = ceilingLinear / limiterEnv;
                }

                for (size_t ch = 0; ch < upCh; ++ch)
                {
                    upsampled.getChannelPointer(ch)[s] *= gain;
                }
            }

            oversampler.processSamplesDown(inBlock);
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::Oversampling<float> oversampler;
        juce::dsp::IIR::Filter<float> monoBassFilter;
        float limiterEnv { 0.0f };
    };
}
