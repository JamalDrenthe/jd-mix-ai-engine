#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

namespace jdmix::core
{
    /**
     * LatencyManager
     * Manages Plugin Delay Compensation (PDC) reporting and provides
     * a latency-matched delay line for processBlockBypassed() so FL Studio
     * never suffers from phase misalignment when bypass is toggled.
     */
    class LatencyManager
    {
    public:
        LatencyManager() = default;

        void prepare(int numChannels, int latencySamples)
        {
            channels = juce::jmax(1, numChannels);
            latency = juce::jmax(0, latencySamples);

            if (latency > 0)
            {
                delayBuffer.setSize(channels, latency + 128);
                delayBuffer.clear();
            }
            writePos = 0;
        }

        int getLatency() const noexcept
        {
            return latency;
        }

        void processBypassed(juce::AudioBuffer<float>& buffer)
        {
            if (latency <= 0)
                return;

            const int numSamples = buffer.getNumSamples();
            const int numCh = juce::jmin(channels, buffer.getNumChannels());
            const int bufLen = delayBuffer.getNumSamples();

            for (int ch = 0; ch < numCh; ++ch)
            {
                const float* src = buffer.getReadPointer(ch);
                float* dst = buffer.getWritePointer(ch);
                float* delayData = delayBuffer.getWritePointer(ch);

                for (int s = 0; s < numSamples; ++s)
                {
                    int writeIndex = (writePos + s) % bufLen;
                    int readIndex = (writePos + s - latency + bufLen) % bufLen;

                    float inVal = src[s];
                    dst[s] = delayData[readIndex];
                    delayData[writeIndex] = inVal;
                }
            }

            writePos = (writePos + numSamples) % bufLen;
        }

    private:
        int channels { 2 };
        int latency { 0 };
        juce::AudioBuffer<float> delayBuffer;
        int writePos { 0 };
    };
}
