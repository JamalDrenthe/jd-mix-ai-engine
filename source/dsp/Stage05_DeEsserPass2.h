#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace jdmix::dsp
{
    /**
     * Stage 5: De-Esser Pass 2 (Post-Comp)
     * Lookahead high-shelf/band polisher (6.5-11.0 kHz, 1-3 dB GR)
     * Tames harshness elevated by Stage 4 compression make-up gain.
     */
    class Stage05_DeEsserPass2
    {
    public:
        Stage05_DeEsserPass2() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;

            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = samplesPerBlock;
            spec.numChannels = 2;

            detectorFilter.prepare(spec);
            smoothedGain.reset(sampleRate, 0.008); // 8 ms smooth lookahead response
            smoothedGain.setCurrentAndTargetValue(1.0f);
        }

        void reset()
        {
            detectorFilter.reset();
            smoothedGain.setCurrentAndTargetValue(1.0f);
        }

        void process(juce::AudioBuffer<float>& buffer, float thresholdDb, float centerFreq)
        {
            if (thresholdDb >= -0.5f) return;

            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();

            *detectorFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                currentSampleRate, centerFreq, 0.7071f, 1.0f);

            juce::AudioBuffer<float> sc(numChannels, numSamples);
            sc.makeCopyOf(buffer);

            juce::dsp::AudioBlock<float> block(sc);
            juce::dsp::ProcessContextReplacing<float> context(block);
            detectorFilter.process(context);

            const float threshLinear = juce::Decibels::decibelsToGain(thresholdDb);

            for (int s = 0; s < numSamples; ++s)
            {
                float peak = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    peak = juce::jmax(peak, std::abs(sc.getSample(ch, s)));

                float target = 1.0f;
                if (peak > threshLinear)
                {
                    float excessDb = juce::Decibels::gainToDecibels(peak / threshLinear);
                    float grDb = -excessDb * 0.45f;
                    grDb = juce::jlimit(-8.0f, 0.0f, grDb);
                    target = juce::Decibels::decibelsToGain(grDb);
                }

                smoothedGain.setTargetValue(target);
                float g = smoothedGain.getNextValue();

                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.setSample(ch, s, buffer.getSample(ch, s) * g);
                }
            }
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> detectorFilter;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGain;
    };
}
