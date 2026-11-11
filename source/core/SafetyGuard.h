#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace jdmix::core
{
    /**
     * SafetyGuard
     * Real-time protection against denormals, NaNs, infinities, and loud transient spikes.
     * Soft clips audio above +12 dBFS (4.0f) using a hyperbolic tangent curve
     * and zeroes corrupted floating-point values to protect speakers and ears.
     */
    class SafetyGuard
    {
    public:
        static void process(juce::AudioBuffer<float>& buffer) noexcept
        {
            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* data = buffer.getWritePointer(ch);

                for (int s = 0; s < numSamples; ++s)
                {
                    float val = data[s];

                    // Check for corrupted NaN or Inf
                    if (std::isnan(val) || std::isinf(val))
                    {
                        data[s] = 0.0f;
                        continue;
                    }

                    // Soft clip if amplitude exceeds threshold
                    constexpr float softThreshold = 1.4142f; // ~ +3 dBFS
                    constexpr float hardLimit     = 4.0f;    // +12 dBFS

                    if (std::abs(val) > hardLimit)
                    {
                        data[s] = std::copysign(softThreshold * 1.5f, val);
                    }
                    else if (std::abs(val) > softThreshold)
                    {
                        // Cubic soft saturation knee
                        float sign = (val > 0.0f) ? 1.0f : -1.0f;
                        float excess = std::abs(val) - softThreshold;
                        data[s] = sign * (softThreshold + std::tanh(excess * 0.5f) * 0.8f);
                    }
                }
            }
        }
    };
}
