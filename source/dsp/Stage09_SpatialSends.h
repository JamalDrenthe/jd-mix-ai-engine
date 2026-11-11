#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 9: Parallel Aux Spatial Sends
     * Room (5-15ms) + Plate/Hall (20-40ms predelay) + Slapback (80-120ms, 0% feedback) + Ducked 3D Throw
     */
    class Stage09_SpatialSends
    {
    public:
        Stage09_SpatialSends() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;

            juce::dsp::Reverb::Parameters params;
            params.roomSize   = 0.65f;
            params.damping    = 0.35f;
            params.wetLevel   = 1.0f;
            params.dryLevel   = 0.0f;
            params.width      = 1.0f;
            reverb.setParameters(params);

            // Delay line for slapback & predelay (up to 500 ms)
            maxDelaySamples = static_cast<int>(sampleRate * 0.5);
            delayRing.setSize(2, maxDelaySamples);
            delayRing.clear();
            delayWritePos = 0;

            duckEnvelope = 0.0f;
        }

        void reset()
        {
            reverb.reset();
            delayRing.clear();
            delayWritePos = 0;
            duckEnvelope = 0.0f;
        }

        void process(juce::AudioBuffer<float>& buffer,
                     float shortRoomSendDb,
                     float plateSendDb, float platePredelayMs,
                     float slapSendDb, float slapTimeMs,
                     float throwSendDb, float throwDuckingDb)
        {
            const int numChannels = juce::jmin(2, buffer.getNumChannels());
            const int numSamples = buffer.getNumSamples();
            if (numSamples == 0 || numChannels == 0) return;

            const float roomGain  = juce::Decibels::decibelsToGain(shortRoomSendDb);
            const float plateGain = juce::Decibels::decibelsToGain(plateSendDb);
            const float slapGain  = juce::Decibels::decibelsToGain(slapSendDb);
            const float throwGain = juce::Decibels::decibelsToGain(throwSendDb);

            // Compute sidechain ducking envelope from dry signal
            float dryPeak = 0.0f;
            for (int s = 0; s < numSamples; ++s)
            {
                float val = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    val = juce::jmax(val, std::abs(buffer.getSample(ch, s)));
                dryPeak = juce::jmax(dryPeak, val);
            }
            duckEnvelope = duckEnvelope * 0.85f + dryPeak * 0.15f;

            // Ducking attenuation
            float duckGain = 1.0f;
            if (duckEnvelope > 0.05f)
            {
                float duckLinear = juce::Decibels::decibelsToGain(-throwDuckingDb);
                duckGain = 1.0f - (1.0f - duckLinear) * juce::jmin(1.0f, duckEnvelope * 4.0f);
            }

            // Write into delay ring
            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* src = buffer.getReadPointer(ch);
                for (int s = 0; s < numSamples; ++s)
                {
                    delayRing.setSample(ch, (delayWritePos + s) % maxDelaySamples, src[s]);
                }
            }

            // 1. Slapback delay (0% feedback, single reflection)
            int slapDelaySamples = static_cast<int>((slapTimeMs * 0.001f) * currentSampleRate);
            slapDelaySamples = juce::jlimit(1, maxDelaySamples - 1, slapDelaySamples);

            // 2. Plate reverb with pre-delay
            juce::AudioBuffer<float> plateWet(numChannels, numSamples);
            int plateDelaySamples = static_cast<int>((platePredelayMs * 0.001f) * currentSampleRate);
            plateDelaySamples = juce::jlimit(1, maxDelaySamples - 1, plateDelaySamples);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* wetData = plateWet.getWritePointer(ch);
                for (int s = 0; s < numSamples; ++s)
                {
                    int rIdx = (delayWritePos + s - plateDelaySamples + maxDelaySamples) % maxDelaySamples;
                    wetData[s] = delayRing.getSample(ch, rIdx) * plateGain;
                }
            }

            juce::dsp::AudioBlock<float> plateBlock(plateWet);
            juce::dsp::ProcessContextReplacing<float> pContext(plateBlock);
            reverb.process(pContext);

            // Sum parallel returns to main buffer
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* dst = buffer.getWritePointer(ch);
                for (int s = 0; s < numSamples; ++s)
                {
                    int slapIdx = (delayWritePos + s - slapDelaySamples + maxDelaySamples) % maxDelaySamples;
                    float slapSample = delayRing.getSample(ch, slapIdx) * slapGain;
                    float plateSample = plateWet.getSample(ch, s) * duckGain;
                    float throwSample = slapSample * throwGain * duckGain;

                    dst[s] += (slapSample + plateSample + throwSample);
                }
            }

            delayWritePos = (delayWritePos + numSamples) % maxDelaySamples;
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::Reverb reverb;
        juce::AudioBuffer<float> delayRing;
        int maxDelaySamples { 22050 };
        int delayWritePos { 0 };
        float duckEnvelope { 0.0f };
    };
}
