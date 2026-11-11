#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 0: Restoration, Denoise/Dereverb Spectral Gate & Breath Control
     */
    class Stage00_Restoration
    {
    public:
        Stage00_Restoration() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;
            envelope = 0.0f;
        }

        void reset()
        {
            envelope = 0.0f;
        }

        void process(juce::AudioBuffer<float>& buffer, float denoiseAmount, float breathCtrlDb)
        {
            if (denoiseAmount <= 0.01f && breathCtrlDb >= -0.1f)
                return;

            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();

            // Downward expander / gate for de-noise & dereverb
            const float threshLinear = juce::Decibels::decibelsToGain(-60.0f + (denoiseAmount * 0.3f));
            const float breathAttenuation = juce::Decibels::decibelsToGain(breathCtrlDb);

            for (int s = 0; s < numSamples; ++s)
            {
                float maxSample = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    maxSample = juce::jmax(maxSample, std::abs(buffer.getSample(ch, s)));

                // Smooth envelope follower (5 ms attack, 50 ms release)
                float coeff = (maxSample > envelope) ? 0.05f : 0.005f;
                envelope = envelope + coeff * (maxSample - envelope);

                float gain = 1.0f;
                if (envelope < threshLinear && denoiseAmount > 0.1f)
                {
                    float ratio = envelope / threshLinear;
                    gain = std::pow(ratio, denoiseAmount * 0.02f);
                }

                // Soft breath dampening
                if (envelope > threshLinear && envelope < threshLinear * 3.0f && breathCtrlDb < -1.0f)
                {
                    gain *= breathAttenuation;
                }

                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.setSample(ch, s, buffer.getSample(ch, s) * gain);
                }
            }
        }

    private:
        double currentSampleRate { 44100.0 };
        float envelope { 0.0f };
    };
}
