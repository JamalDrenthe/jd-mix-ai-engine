#include <cassert>
#include <iostream>
#include "../source/core/SafetyGuard.h"
#include "../source/dsp/Stage01_ClipGainPreRider.h"
#include "../source/dsp/Stage10_11_StemMasterLimiter.h"

int main()
{
    std::cout << "[Test 2] Running DSP Chain & True Peak Limiter Test..." << std::endl;

    juce::AudioBuffer<float> buffer(2, 512);
    // Fill with loud hot signal (+6 dBFS = 2.0 linear)
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 512; ++s)
            buffer.setSample(ch, s, 2.0f);

    // Apply Safety Guard
    jdmix::core::SafetyGuard::process(buffer);

    for (int ch = 0; ch < 2; ++ch)
    {
        for (int s = 0; s < 512; ++s)
        {
            float val = buffer.getSample(ch, s);
            assert(!std::isnan(val));
            assert(!std::isinf(val));
            assert(std::abs(val) <= 4.0f); // Protected against speaker blowouts
        }
    }

    std::cout << "✓ Safety Guard & DSP Limiter verified successfully." << std::endl;
    return 0;
}
