#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace jdmix::dsp
{
    /**
     * Stage 8: Additive Tone-Shaping EQ (Post-Sat)
     * Pultec Warmth (100-250 Hz), Presence (2-5 kHz), Silk Air (8-16 kHz)
     */
    class Stage08_AdditiveToneEq
    {
    public:
        Stage08_AdditiveToneEq() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;

            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = samplesPerBlock;
            spec.numChannels = 2;

            warmthFilter.prepare(spec);
            presenceFilter.prepare(spec);
            airFilter.prepare(spec);

            updateFilters(160.0f, 0.0f, 3500.0f, 0.0f, 12000.0f, 0.0f);
        }

        void reset()
        {
            warmthFilter.reset();
            presenceFilter.reset();
            airFilter.reset();
        }

        void updateFilters(float warmthFreq, float warmthGainDb,
                           float presenceFreq, float presenceGainDb,
                           float airFreq, float airGainDb)
        {
            *warmthFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate, warmthFreq, 1.0f, juce::Decibels::decibelsToGain(warmthGainDb));

            *presenceFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate, presenceFreq, 1.4f, juce::Decibels::decibelsToGain(presenceGainDb));

            *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                currentSampleRate, airFreq, 0.7071f, juce::Decibels::decibelsToGain(airGainDb));
        }

        void process(juce::AudioBuffer<float>& buffer,
                     float warmthFreq, float warmthGainDb,
                     float presenceFreq, float presenceGainDb,
                     float airFreq, float airGainDb)
        {
            updateFilters(warmthFreq, warmthGainDb, presenceFreq, presenceGainDb, airFreq, airGainDb);

            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);

            warmthFilter.process(context);
            presenceFilter.process(context);
            airFilter.process(context);
        }

    private:
        double currentSampleRate { 44100.0 };
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> warmthFilter;
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> presenceFilter;
        juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> airFilter;
    };
}
