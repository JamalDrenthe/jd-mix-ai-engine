#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 1: Clip Gain (-18 dBFS) & Pre-Comp Vocal Rider
     * Levels wide 15-20 dB dynamic jumps before reaching the analog compressors.
     */
    class Stage01_ClipGainPreRider
    {
    public:
        Stage01_ClipGainPreRider() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;
            riderGain.reset(sampleRate, 0.050); // 50 ms smoothing
            riderGain.setCurrentAndTargetValue(1.0f);
            rmsLevel = 0.01f;
        }

        void reset()
        {
            riderGain.setCurrentAndTargetValue(1.0f);
            rmsLevel = 0.01f;
        }

        void process(juce::AudioBuffer<float>& buffer, float clipTrimDb, float preRiderTargetDb)
        {
            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();
            if (numSamples == 0) return;

            // Apply static clip trim
            const float trimGain = juce::Decibels::decibelsToGain(clipTrimDb);
            buffer.applyGain(trimGain);

            // Compute block RMS for rider
            float currentRms = buffer.getRMSLevel(0, 0, numSamples);
            if (numChannels > 1)
                currentRms = juce::jmax(currentRms, buffer.getRMSLevel(1, 0, numSamples));

            // Smooth RMS tracker
            rmsLevel = rmsLevel * 0.95f + currentRms * 0.05f;

            // Target RMS: -18 dBFS + user preRiderTargetDb offset
            const float targetRms = juce::Decibels::decibelsToGain(-18.0f + preRiderTargetDb);

            if (rmsLevel > 0.001f) // Signal present
            {
                float neededGain = targetRms / rmsLevel;
                // Clamp rider action to max ±8 dB
                neededGain = juce::jlimit(0.398f, 2.51f, neededGain);
                riderGain.setTargetValue(neededGain);
            }
            else
            {
                riderGain.setTargetValue(1.0f);
            }

            // Smooth sample-by-sample application to prevent clicks
            for (int s = 0; s < numSamples; ++s)
            {
                float g = riderGain.getNextValue();
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.setSample(ch, s, buffer.getSample(ch, s) * g);
                }
            }
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> riderGain;
        float rmsLevel { 0.01f };
    };
}
