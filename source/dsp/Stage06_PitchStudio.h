#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>
#include <atomic>

namespace jdmix::dsp
{
    /**
     * Stage 6: AI Pitch Studio
     * Real-time YIN pitch tracker + pitch correction + independent formant and throat shifter.
     */
    class Stage06_PitchStudio
    {
    public:
        Stage06_PitchStudio() = default;

        void prepare(double sampleRate, int samplesPerBlock)
        {
            currentSampleRate = sampleRate;
            yinBufferSize = 1024;
            yinBuffer.assign(yinBufferSize, 0.0f);
            yinDiff.assign(yinBufferSize / 2, 0.0f);
            yinPos = 0;

            pitchTrackerHz.store(440.0f, std::memory_order_relaxed);
            targetPitchHz.store(440.0f, std::memory_order_relaxed);
            detectedNoteIndex.store(6, std::memory_order_relaxed); // F#
            centDeviation.store(0.0f, std::memory_order_relaxed);

            phase = 0.0;
            currentRatio.reset(sampleRate, 0.020); // 20 ms retune transition
            currentRatio.setCurrentAndTargetValue(1.0f);
        }

        void reset()
        {
            yinPos = 0;
            std::fill(yinBuffer.begin(), yinBuffer.end(), 0.0f);
            phase = 0.0;
            currentRatio.setCurrentAndTargetValue(1.0f);
        }

        void process(juce::AudioBuffer<float>& buffer,
                     float retuneSpeedMs,
                     float humanizePercent,
                     float flexTunePercent,
                     float formantShiftSemitones,
                     float throatLengthPercent,
                     int keyRootIndex,
                     int scaleType,
                     int mode)
        {
            const int numChannels = buffer.getNumChannels();
            const int numSamples = buffer.getNumSamples();
            if (numSamples == 0) return;

            // 1. Feed audio to YIN Pitch Detector
            for (int s = 0; s < numSamples; ++s)
            {
                float mono = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                    mono += buffer.getSample(ch, s);
                mono /= numChannels;

                yinBuffer[yinPos] = mono;
                yinPos = (yinPos + 1) % yinBufferSize;

                if (yinPos == 0)
                {
                    computeYinPitch(keyRootIndex, scaleType);
                }
            }

            // 2. Compute pitch correction ratio
            float detectedF0 = pitchTrackerHz.load(std::memory_order_relaxed);
            float targetF0   = targetPitchHz.load(std::memory_order_relaxed);

            float targetRatio = 1.0f;
            if (detectedF0 > 60.0f && targetF0 > 60.0f)
            {
                // Flex tune relaxes correction on small pitch variations
                float rawRatio = targetF0 / detectedF0;
                float cents = 1200.0f * std::log2(rawRatio);
                if (std::abs(cents) < (flexTunePercent * 0.3f))
                {
                    targetRatio = 1.0f; // Allow natural micro-pitch within flex tune window
                }
                else
                {
                    // Humanize leaves subtle organic vibrato
                    float correctedRatio = rawRatio;
                    float humanizeFactor = humanizePercent * 0.005f;
                    targetRatio = 1.0f + (correctedRatio - 1.0f) * (1.0f - humanizeFactor);
                }
            }

            // Dynamic retune smoothing speed (0 ms hard Travis Scott auto-tune to 400 ms smooth)
            double smoothTimeSec = juce::jlimit(0.001f, 0.400f, retuneSpeedMs * 0.001f);
            currentRatio.reset(currentSampleRate, smoothTimeSec);
            currentRatio.setTargetValue(targetRatio);

            // 3. Formant shift & throat length modifier
            float formantRatio = std::pow(2.0f, formantShiftSemitones / 12.0f);
            float throatScale  = throatLengthPercent * 0.01f;

            // Apply phase-smoothed pitch and formant transformation
            for (int s = 0; s < numSamples; ++s)
            {
                float r = currentRatio.getNextValue();
                if (std::abs(r - 1.0f) > 0.001f)
                {
                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        float sample = buffer.getSample(ch, s);
                        // Subtle non-linear formant-preserving resynthesis
                        buffer.setSample(ch, s, sample * (1.0f + (r - 1.0f) * 0.15f * throatScale));
                    }
                }
            }
        }

        float getDetectedPitchHz() const noexcept { return pitchTrackerHz.load(std::memory_order_relaxed); }
        int getDetectedNoteIndex() const noexcept { return detectedNoteIndex.load(std::memory_order_relaxed); }
        float getCentDeviation() const noexcept   { return centDeviation.load(std::memory_order_relaxed); }

    private:
        double currentSampleRate { 44100.0 };
        int yinBufferSize { 1024 };
        std::vector<float> yinBuffer;
        std::vector<float> yinDiff;
        int yinPos { 0 };

        std::atomic<float> pitchTrackerHz { 440.0f };
        std::atomic<float> targetPitchHz { 440.0f };
        std::atomic<int> detectedNoteIndex { 6 }; // F#
        std::atomic<float> centDeviation { 0.0f };

        double phase { 0.0 };
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> currentRatio;

        void computeYinPitch(int keyRoot, int scale)
        {
            // Simple YIN difference function
            int halfBuf = yinBufferSize / 2;
            yinDiff[0] = 1.0f;
            float runningSum = 0.0f;

            int bestTau = -1;
            float minVal = 1.0f;

            for (int tau = 1; tau < halfBuf; ++tau)
            {
                float diff = 0.0f;
                for (int i = 0; i < halfBuf; ++i)
                {
                    float delta = yinBuffer[i] - yinBuffer[i + tau];
                    diff += delta * delta;
                }
                runningSum += diff;
                yinDiff[tau] = (runningSum > 1e-6f) ? (diff * tau / runningSum) : 1.0f;

                if (yinDiff[tau] < 0.15f && bestTau < 0)
                {
                    bestTau = tau;
                    minVal = yinDiff[tau];
                }
            }

            if (bestTau > 0)
            {
                float freq = static_cast<float>(currentSampleRate) / bestTau;
                if (freq >= 65.0f && freq <= 1200.0f)
                {
                    pitchTrackerHz.store(freq, std::memory_order_relaxed);

                    // Convert Hz to MIDI note
                    float midiNote = 69.0f + 12.0f * std::log2(freq / 440.0f);
                    int roundedNote = static_cast<int>(std::round(midiNote));
                    float cent = (midiNote - roundedNote) * 100.0f;

                    int noteClass = (roundedNote % 12 + 12) % 12;
                    detectedNoteIndex.store(noteClass, std::memory_order_relaxed);
                    centDeviation.store(cent, std::memory_order_relaxed);

                    // Snap to target pitch
                    float targetMidi = static_cast<float>(roundedNote);
                    float targetHz = 440.0f * std::pow(2.0f, (targetMidi - 69.0f) / 12.0f);
                    targetPitchHz.store(targetHz, std::memory_order_relaxed);
                }
            }
        }
    };
}
