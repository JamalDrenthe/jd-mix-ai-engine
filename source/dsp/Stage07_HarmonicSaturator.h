#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 7: Harmonic Saturation (Tube / Tape / Console)
     * 4x Polyphase Oversampled non-linear saturation for anti-aliased analog warmth.
     */
    class Stage07_HarmonicSaturator
    {
    public:
        Stage07_HarmonicSaturator()
            : oversampler(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)
        {
        }

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;
            oversampler.initProcessing(samplesPerBlock);
            reset();
        }

        void reset()
        {
            oversampler.reset();
        }

        void process(juce::AudioBuffer<float>& buffer, int satType, float drivePercent, float mixPercent)
        {
            if (drivePercent <= 0.5f || mixPercent <= 0.5f) return;

            const int numSamples = buffer.getNumSamples();
            const float mix = mixPercent * 0.01f;
            const float driveGain = 1.0f + (drivePercent * 0.05f); // 1x to 6x drive

            juce::AudioBuffer<float> wet(buffer.getNumChannels(), numSamples);
            wet.makeCopyOf(buffer);

            // 4x Oversample Up
            juce::dsp::AudioBlock<float> inBlock(wet);
            auto upsampledBlock = oversampler.processSamplesUp(inBlock);

            const size_t upChannels = upsampledBlock.getNumChannels();
            const size_t upSamples = upsampledBlock.getNumSamples();

            for (size_t ch = 0; ch < upChannels; ++ch)
            {
                float* chData = upsampledBlock.getChannelPointer(ch);

                for (size_t s = 0; s < upSamples; ++s)
                {
                    float x = chData[s] * driveGain;

                    if (satType == 0) // Tube: asymmetric 2nd & 3rd harmonic
                    {
                        if (x >= 0.0f)
                            chData[s] = std::tanh(x) * 0.9f;
                        else
                            chData[s] = (std::tanh(x * 1.2f) + x * 0.15f) * 0.85f;
                    }
                    else if (satType == 1) // Tape: soft compression & 3rd harmonic saturation
                    {
                        chData[s] = (1.5f * x - 0.5f * x * x * x * 0.33f) / (1.0f + std::abs(x) * 0.4f);
                    }
                    else // Console: transformer saturation & subtle even-order warmth
                    {
                        chData[s] = std::tanh(x * 0.95f) + 0.05f * x * x;
                    }
                }
            }

            // Downsample back to base rate
            oversampler.processSamplesDown(inBlock);

            // Wet/Dry mix
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int s = 0; s < numSamples; ++s)
                {
                    float dryVal = buffer.getSample(ch, s);
                    float wetVal = wet.getSample(ch, s) / driveGain;
                    buffer.setSample(ch, s, dryVal * (1.0f - mix) + wetVal * mix);
                }
            }
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::Oversampling<float> oversampler;
    };
}
