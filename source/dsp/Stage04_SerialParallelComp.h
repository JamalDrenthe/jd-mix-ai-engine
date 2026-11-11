#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 4: Serial (Opto + FET) & Parallel Compression
     * 4A: Opto (LA-2A style) musical macro-leveling
     * 4B: FET (1176 style) ultra-fast peak and transient catcher
     * 4C: Parallel upward crush for dense modern commercial tone
     */
    class Stage04_SerialParallelComp
    {
    public:
        Stage04_SerialParallelComp() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;
            optoEnv = 0.0f;
            fetEnv = 0.0f;
            parEnv = 0.0f;
        }

        void reset()
        {
            optoEnv = 0.0f;
            fetEnv = 0.0f;
            parEnv = 0.0f;
        }

        float process(juce::AudioBuffer<float>& buffer,
                      float optoThreshDb, float optoGainDb,
                      float fetThreshDb, int fetRatioIdx, float fetAttackMs, float fetReleaseMs,
                      float parBlendPercent)
        {
            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();
            if (numSamples == 0) return 0.0f;

            // Keep copy for parallel compression path (4C)
            juce::AudioBuffer<float> parallelBuffer;
            if (parBlendPercent > 0.01f)
            {
                parallelBuffer.makeCopyOf(buffer);
            }

            float maxGainReductionDb = 0.0f;

            // 1. Opto Compressor (LA-2A model)
            const float optoThreshLinear = juce::Decibels::decibelsToGain(optoThreshDb);
            const float optoMakeup = juce::Decibels::decibelsToGain(optoGainDb);

            // Program-dependent Opto time constants
            const float optoAttackCoeff  = std::exp(-1.0f / (0.010f * currentSampleRate)); // 10 ms attack
            const float optoReleaseCoeff = std::exp(-1.0f / (0.150f * currentSampleRate)); // 150 ms release

            // 2. FET Compressor (1176 model)
            const float fetThreshLinear = juce::Decibels::decibelsToGain(fetThreshDb);
            float fetRatio = 4.0f;
            if (fetRatioIdx == 1) fetRatio = 8.0f;
            else if (fetRatioIdx == 2) fetRatio = 12.0f;
            else if (fetRatioIdx == 3) fetRatio = 20.0f;

            const float fetAttackCoeff  = std::exp(-1.0f / ((fetAttackMs * 0.001f) * currentSampleRate));
            const float fetReleaseCoeff = std::exp(-1.0f / ((fetReleaseMs * 0.001f) * currentSampleRate));

            for (int s = 0; s < numSamples; ++s)
            {
                float peakSample = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    peakSample = juce::jmax(peakSample, std::abs(buffer.getSample(ch, s)));

                // Opto detector
                if (peakSample > optoEnv)
                    optoEnv = peakSample + optoAttackCoeff * (optoEnv - peakSample);
                else
                    optoEnv = peakSample + optoReleaseCoeff * (optoEnv - peakSample);

                float optoGain = 1.0f;
                if (optoEnv > optoThreshLinear && optoThreshDb < -0.1f)
                {
                    float excessDb = juce::Decibels::gainToDecibels(optoEnv / optoThreshLinear);
                    float grDb = -excessDb * 0.5f; // ~3:1 ratio
                    optoGain = juce::Decibels::decibelsToGain(grDb);
                    maxGainReductionDb = juce::jmin(maxGainReductionDb, grDb);
                }

                // Apply Opto
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.setSample(ch, s, buffer.getSample(ch, s) * optoGain * optoMakeup);
                }

                // FET detector
                float fetInSample = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    fetInSample = juce::jmax(fetInSample, std::abs(buffer.getSample(ch, s)));

                if (fetInSample > fetEnv)
                    fetEnv = fetInSample + fetAttackCoeff * (fetEnv - fetInSample);
                else
                    fetEnv = fetInSample + fetReleaseCoeff * (fetEnv - fetInSample);

                float fetGain = 1.0f;
                if (fetEnv > fetThreshLinear && fetThreshDb < -0.1f)
                {
                    float excessDb = juce::Decibels::gainToDecibels(fetEnv / fetThreshLinear);
                    float grDb = -excessDb * (1.0f - 1.0f / fetRatio);
                    fetGain = juce::Decibels::decibelsToGain(grDb);
                    maxGainReductionDb = juce::jmin(maxGainReductionDb, grDb);
                }

                // Apply FET
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.setSample(ch, s, buffer.getSample(ch, s) * fetGain);
                }
            }

            // 3. Parallel Crush Path (4C)
            if (parBlendPercent > 0.01f)
            {
                const float parBlend = parBlendPercent * 0.01f;
                const float parThreshLinear = juce::Decibels::decibelsToGain(-30.0f); // Aggressive threshold
                const float parAttackCoeff = std::exp(-1.0f / (0.0002f * currentSampleRate)); // 0.2 ms
                const float parReleaseCoeff = std::exp(-1.0f / (0.040f * currentSampleRate)); // 40 ms

                for (int s = 0; s < numSamples; ++s)
                {
                    float drySample = 0.0f;
                    for (int ch = 0; ch < numChannels; ++ch)
                        drySample = juce::jmax(drySample, std::abs(parallelBuffer.getSample(ch, s)));

                    if (drySample > parEnv)
                        parEnv = drySample + parAttackCoeff * (parEnv - drySample);
                    else
                        parEnv = drySample + parReleaseCoeff * (parEnv - drySample);

                    float crushGain = 1.0f;
                    if (parEnv > parThreshLinear)
                    {
                        float excessDb = juce::Decibels::gainToDecibels(parEnv / parThreshLinear);
                        crushGain = juce::Decibels::decibelsToGain(-excessDb * 0.916f); // 12:1
                    }

                    // Makeup gain on parallel crush
                    crushGain *= 3.16f; // +10 dB makeup

                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        float dry = buffer.getSample(ch, s);
                        float crushed = parallelBuffer.getSample(ch, s) * crushGain;
                        buffer.setSample(ch, s, dry * (1.0f - parBlend * 0.5f) + crushed * parBlend);
                    }
                }
            }

            return maxGainReductionDb;
        }

    private:
        double currentSampleRate { 44100.0 };
        float optoEnv { 0.0f };
        float fetEnv { 0.0f };
        float parEnv { 0.0f };
    };
}
