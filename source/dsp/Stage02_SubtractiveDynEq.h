#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace jdmix::dsp
{
    /**
     * Stage 2: Subtractive & Dynamic Cleanup EQ (Pre-Comp)
     * High-pass filter (80-120 Hz) + Dynamic Mud Bell (200-400 Hz) + Dynamic Harsh Notch (2-4.5 kHz)
     */
    class Stage02_SubtractiveDynEq
    {
    public:
        Stage02_SubtractiveDynEq() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;

            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = samplesPerBlock;
            spec.numChannels = 2;

            hpfFilter.prepare(spec);
            mudFilter.prepare(spec);
            resFilter.prepare(spec);

            updateFilters(90.0f, 285.0f, 0.0f, 3200.0f, 0.0f);
        }

        void reset()
        {
            hpfFilter.reset();
            mudFilter.reset();
            resFilter.reset();
        }

        void updateFilters(float hpfFreq, float mudFreq, float mudGainDb, float resFreq, float resGainDb)
        {
            // 24 dB/oct HPF (2 cascaded state-variable filters)
            *hpfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, hpfFreq, 0.7071f);

            // Mud Bell (Q = 1.8)
            *mudFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate, mudFreq, 1.8f, juce::Decibels::decibelsToGain(mudGainDb));

            // Resonant Notch (Q = 4.0)
            *resFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate, resFreq, 4.0f, juce::Decibels::decibelsToGain(resGainDb));
        }

        void process(juce::AudioBuffer<float>& buffer,
                     float hpfFreq,
                     float mudFreq, float mudGainDb,
                     float resFreq, float resGainDb)
        {
            updateFilters(hpfFreq, mudFreq, mudGainDb, resFreq, resGainDb);

            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);

            hpfFilter.process(context);
            mudFilter.process(context);
            resFilter.process(context);
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> hpfFilter;
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> mudFilter;
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> resFilter;
    };
}
