#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <algorithm>
#include <cstring>

namespace jdmix::core
{
    /**
     * VariableBlockFifo
     * Solves FL Studio Fruity Wrapper variable buffer sizing.
     * Buffers arbitrary incoming blocks (e.g. 17, 43, 128, 384 samples)
     * and guarantees deterministic fixed-size processing blocks (default 1024).
     * Maintains sample-accurate Plugin Delay Compensation (PDC) alignment.
     */
    template <typename FloatType = float>
    class VariableBlockFifo
    {
    public:
        VariableBlockFifo() = default;

        void prepare(int numChannels, int targetBlockSize, int maxFifoCapacity = 16384)
        {
            channels = juce::jmax(1, numChannels);
            fixedBlockSize = juce::jmax(64, targetBlockSize);
            fifoCapacity = juce::jmax(maxFifoCapacity, fixedBlockSize * 8);

            inputRing.setSize(channels, fifoCapacity);
            outputRing.setSize(channels, fifoCapacity);
            currentFixedBlock.setSize(channels, fixedBlockSize);

            reset();
        }

        void reset()
        {
            inputRing.clear();
            outputRing.clear();
            currentFixedBlock.clear();

            inputWritePos = 0;
            inputReadPos = 0;
            outputWritePos = 0;
            outputReadPos = 0;
            numSamplesAvailable = 0;
            numProcessedAvailable = 0;

            // Pre-fill output FIFO with latency silence to establish deterministic PDC offset
            if (fifoCapacity > 0 && fixedBlockSize > 0)
            {
                outputWritePos = fixedBlockSize;
                numProcessedAvailable = fixedBlockSize;
            }
        }

        int getLatencySamples() const noexcept
        {
            return fixedBlockSize;
        }

        int getFixedBlockSize() const noexcept
        {
            return fixedBlockSize;
        }

        void writeSamples(const juce::AudioBuffer<FloatType>& input)
        {
            const int numSamples = input.getNumSamples();
            const int numCh = juce::jmin(channels, input.getNumChannels());

            for (int ch = 0; ch < numCh; ++ch)
            {
                const FloatType* src = input.getReadPointer(ch);
                int writeIdx = inputWritePos;

                for (int s = 0; s < numSamples; ++s)
                {
                    inputRing.setSample(ch, (writeIdx + s) % fifoCapacity, src[s]);
                }
            }

            inputWritePos = (inputWritePos + numSamples) % fifoCapacity;
            numSamplesAvailable += numSamples;
        }

        bool hasBlockReady() const noexcept
        {
            return numSamplesAvailable >= fixedBlockSize;
        }

        juce::AudioBuffer<FloatType>& getReadyBlock()
        {
            for (int ch = 0; ch < channels; ++ch)
            {
                FloatType* dest = currentFixedBlock.getWritePointer(ch);
                int readIdx = inputReadPos;

                for (int s = 0; s < fixedBlockSize; ++s)
                {
                    dest[s] = inputRing.getSample(ch, (readIdx + s) % fifoCapacity);
                }
            }

            return currentFixedBlock;
        }

        void blockProcessed()
        {
            // Advance input read pos
            inputReadPos = (inputReadPos + fixedBlockSize) % fifoCapacity;
            numSamplesAvailable -= fixedBlockSize;

            // Copy processed block to output ring
            for (int ch = 0; ch < channels; ++ch)
            {
                const FloatType* src = currentFixedBlock.getReadPointer(ch);
                int writeIdx = outputWritePos;

                for (int s = 0; s < fixedBlockSize; ++s)
                {
                    outputRing.setSample(ch, (writeIdx + s) % fifoCapacity, src[s]);
                }
            }

            outputWritePos = (outputWritePos + fixedBlockSize) % fifoCapacity;
            numProcessedAvailable += fixedBlockSize;
        }

        void readProcessedSamples(juce::AudioBuffer<FloatType>& output)
        {
            const int numSamples = output.getNumSamples();
            const int numCh = juce::jmin(channels, output.getNumChannels());

            for (int ch = 0; ch < numCh; ++ch)
            {
                FloatType* dest = output.getWritePointer(ch);
                int readIdx = outputReadPos;

                for (int s = 0; s < numSamples; ++s)
                {
                    if (numProcessedAvailable > s)
                    {
                        dest[s] = outputRing.getSample(ch, (readIdx + s) % fifoCapacity);
                    }
                    else
                    {
                        dest[s] = 0.0f; // Silence fallback if underrun
                    }
                }
            }

            int toAdvance = juce::jmin(numSamples, numProcessedAvailable);
            outputReadPos = (outputReadPos + toAdvance) % fifoCapacity;
            numProcessedAvailable -= toAdvance;
        }

    private:
        int channels { 2 };
        int fixedBlockSize { 1024 };
        int fifoCapacity { 16384 };

        juce::AudioBuffer<FloatType> inputRing;
        juce::AudioBuffer<FloatType> outputRing;
        juce::AudioBuffer<FloatType> currentFixedBlock;

        int inputWritePos { 0 };
        int inputReadPos { 0 };
        int outputWritePos { 0 };
        int outputReadPos { 0 };

        int numSamplesAvailable { 0 };
        int numProcessedAvailable { 0 };
    };
}
