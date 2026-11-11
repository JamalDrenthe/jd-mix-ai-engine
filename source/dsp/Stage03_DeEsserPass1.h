#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 3: De-Esser Pass 1 (Pre-Comp)
     * Split-band fast detector (5-9 kHz) to prevent S-consonants from triggering Stage 4 compressors.
     */
    class Stage03_DeEsserPass1
    {
    public:
        Stage03_DeEsserPass1() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;

            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = samplesPerBlock;
            spec.numChannels = 2;

            sidechainBandpass.prepare(spec);
            reductionGain.reset(sampleRate, 0.005); // 5 ms fast attack/release
            reductionGain.setCurrentAndTargetValue(1.0f);
        }

        void reset()
        {
            sidechainBandpass.reset();
            reductionGain.setCurrentAndTargetValue(1.0f);
        }

        void process(juce::AudioBuffer<float>& buffer, float thresholdDb, float centerFreq)
        {
            if (thresholdDb >= -0.5f) return; // Bypass if threshold near zero

            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();

            // Configure sidechain detector filter
            *sidechainBandpass.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(
                currentSampleRate, centerFreq, 2.5f);

            juce::AudioBuffer<float> scBuffer;
            scBuffer.makeCopyOf(buffer);

            juce::dsp::AudioBlock<float> block(scBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            sidechainBandpass.process(context);

            const float threshLinear = juce::Decibels::decibelsToGain(thresholdDb);

            for (int s = 0; s < numSamples; ++s)
            {
                float scSample = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    scSample = juce::jmax(scSample, std::abs(scBuffer.getSample(ch, s)));

                float targetGain = 1.0f;
                if (scSample > threshLinear)
                {
                    float excessDb = juce::Decibels::gainToDecibels(scSample / threshLinear);
                    float grDb = -excessDb * 0.6f; // 2.5:1 ratio
                    grDb = juce::jlimit(-12.0f, 0.0f, grDb);
                    targetGain = juce::Decibels::decibelsToGain(grDb);
                }

                reductionGain.setTargetValue(targetGain);
                float g = reductionGain.getNextValue();

                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.setSample(ch, s, buffer.getSample(ch, s) * g);
                }
            }
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> sidechainBandpass;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reductionGain;
    };
}
