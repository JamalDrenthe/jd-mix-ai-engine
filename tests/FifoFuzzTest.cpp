#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include "../source/core/VariableBlockFifo.h"

int main()
{
    std::cout << "[Test 1] Running VariableBlockFifo Fuzzing Test for FL Studio..." << std::endl;

    jdmix::core::VariableBlockFifo<float> fifo;
    fifo.prepare(2, 1024);

    assert(fifo.getLatencySamples() == 1024);

    // Feed arbitrary chunks like FL Studio (17, 43, 128, 384 samples)
    const std::vector<int> chunkSizes = { 17, 43, 128, 384, 512, 17, 99, 1024, 73 };
    int totalPushed = 0;
    int totalRead = 0;

    juce::AudioBuffer<float> tempIn;
    juce::AudioBuffer<float> tempOut;

    for (int chunk : chunkSizes)
    {
        tempIn.setSize(2, chunk);
        tempOut.setSize(2, chunk);

        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < chunk; ++s)
                tempIn.setSample(ch, s, 0.5f);

        fifo.writeSamples(tempIn);
        totalPushed += chunk;

        while (fifo.hasBlockReady())
        {
            auto& block = fifo.getReadyBlock();
            assert(block.getNumSamples() == 1024);
            // Invert or process block
            fifo.blockProcessed();
        }

        fifo.readProcessedSamples(tempOut);
        totalRead += chunk;
    }

    assert(totalPushed == totalRead);
    std::cout << "✓ VariableBlockFifo Fuzzing Passed: Processed " << totalPushed << " samples in arbitrary chunks." << std::endl;
    return 0;
}
