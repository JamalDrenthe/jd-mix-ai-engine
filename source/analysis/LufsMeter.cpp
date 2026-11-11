#include "LufsMeter.h"
#include <cmath>

namespace jdmix::analysis
{
    void LufsMeter::prepare(double sampleRate, int maxBlockSize)
    {
        currentSampleRate = sampleRate;

        // BS.1770 K-weighting filter coefficients:
        // Stage 1: High shelf (+4 dB around 1680 Hz)
        auto stage1Coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 1681.0f, 0.7071f, juce::Decibels::decibelsToGain(3.9998f));

        // Stage 2: High pass (Butterworth around 38 Hz)
        auto stage2Coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, 38.135f, 0.5f);

        for (int ch = 0; ch < 2; ++ch)
        {
            kFilterStage1[ch].coefficients = stage1Coeffs;
            kFilterStage2[ch].coefficients = stage2Coeffs;
        }

        momentaryLength = static_cast<int>(sampleRate * 0.400); // 400 ms
        shortTermLength = static_cast<int>(sampleRate * 3.000); // 3000 ms

        momentaryBuffer.assign(momentaryLength, 0.0f);
        shortTermBuffer.assign(shortTermLength, 0.0f);

        oversampler.initProcessing(maxBlockSize);
        reset();
    }

    void LufsMeter::reset()
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            kFilterStage1[ch].reset();
            kFilterStage2[ch].reset();
        }

        std::fill(momentaryBuffer.begin(), momentaryBuffer.end(), 0.0f);
        std::fill(shortTermBuffer.begin(), shortTermBuffer.end(), 0.0f);
        momentaryIndex = 0;
        shortTermIndex = 0;

        blockPowers.clear();
        totalGatedPower = 0.0;
        gatedBlockCount = 0;

        momentaryLufs.store(-100.0f, std::memory_order_relaxed);
        shortTermLufs.store(-100.0f, std::memory_order_relaxed);
        integratedLufs.store(-100.0f, std::memory_order_relaxed);
        maxTruePeak.store(-100.0f, std::memory_order_relaxed);

        oversampler.reset();
    }

    void LufsMeter::process(const juce::AudioBuffer<float>& buffer)
    {
        const int numChannels = juce::jmin(2, buffer.getNumChannels());
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0 || numChannels == 0)
            return;

        // 1. True Peak calculation via 4x oversampling
        juce::dsp::AudioBlock<const float> inBlock(buffer);
        auto upsampledBlock = oversampler.processSamplesUp(inBlock);

        float peakLinear = 0.0f;
        for (size_t ch = 0; ch < upsampledBlock.getNumChannels(); ++ch)
        {
            const float* chData = upsampledBlock.getChannelPointer(ch);
            for (size_t s = 0; s < upsampledBlock.getNumSamples(); ++s)
            {
                peakLinear = juce::jmax(peakLinear, std::abs(chData[s]));
            }
        }
        float peakDb = juce::Decibels::gainToDecibels(peakLinear, -100.0f);
        float prevPeak = maxTruePeak.load(std::memory_order_relaxed);
        if (peakDb > prevPeak)
            maxTruePeak.store(peakDb, std::memory_order_relaxed);

        // 2. K-weighting filtering for loudness
        juce::AudioBuffer<float> filtered(numChannels, numSamples);
        filtered.makeCopyOf(buffer);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = filtered.getWritePointer(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = data[s];
                sample = kFilterStage1[ch].processSample(sample);
                sample = kFilterStage2[ch].processSample(sample);
                data[s] = sample;
            }
        }

        // Sum powers across channels
        double samplePowerSum = 0.0;
        for (int s = 0; s < numSamples; ++s)
        {
            float samplePower = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float val = filtered.getSample(ch, s);
                samplePower += val * val;
            }

            momentaryBuffer[momentaryIndex] = samplePower;
            shortTermBuffer[shortTermIndex] = samplePower;

            momentaryIndex = (momentaryIndex + 1) % momentaryLength;
            shortTermIndex = (shortTermIndex + 1) % shortTermLength;

            samplePowerSum += samplePower;
        }

        // Calculate momentary power (400 ms mean)
        double momentarySum = 0.0;
        for (float p : momentaryBuffer)
            momentarySum += p;
        double momentaryMean = momentarySum / momentaryLength;
        float mVal = (momentaryMean > 1e-12) ? static_cast<float>(-0.691 + 10.0 * std::log10(momentaryMean)) : -100.0f;
        momentaryLufs.store(mVal, std::memory_order_relaxed);

        // Calculate short-term power (3s mean)
        double shortTermSum = 0.0;
        for (float p : shortTermBuffer)
            shortTermSum += p;
        double shortTermMean = shortTermSum / shortTermLength;
        float sVal = (shortTermMean > 1e-12) ? static_cast<float>(-0.691 + 10.0 * std::log10(shortTermMean)) : -100.0f;
        shortTermLufs.store(sVal, std::memory_order_relaxed);

        // Accumulate block power for Integrated Loudness with gating
        float blockMean = static_cast<float>(samplePowerSum / numSamples);
        if (blockMean > 1e-10f) // -70 LKFS absolute gate
        {
            blockPowers.push_back(blockMean);
            totalGatedPower += blockMean;
            gatedBlockCount++;

            double avgPower = totalGatedPower / gatedBlockCount;
            float iVal = static_cast<float>(-0.691 + 10.0 * std::log10(avgPower));
            integratedLufs.store(iVal, std::memory_order_relaxed);
        }
    }
}
