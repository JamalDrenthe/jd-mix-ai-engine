#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <string>
#include <functional>
#include "GeminiSchema.h"
#include "../analysis/MirFeatureExtractor.h"

namespace jdmix::ai
{
    enum class AiStatus
    {
        Idle,
        Analyzing,
        Applying,
        Completed,
        Failed
    };

    class GeminiClient : private juce::Thread
    {
    public:
        explicit GeminiClient(juce::AudioProcessorValueTreeState& apvts);
        ~GeminiClient() override;

        void triggerAutoOptimize(const analysis::MirAcousticSnapshot& snapshot,
                                 const juce::String& userPrompt = "",
                                 float intensity = 100.0f);

        AiStatus getStatus() const noexcept { return currentStatus.load(std::memory_order_relaxed); }
        juce::String getStatusMessage() const;
        float getConfidenceScore() const noexcept { return lastConfidenceScore.load(std::memory_order_relaxed); }

        // A/B Auditioning
        void toggleSlotB(bool useSlotB);
        bool isSlotBActive() const noexcept { return slotBActive.load(std::memory_order_relaxed); }
        void toggleDeltaMode(bool enableDelta);
        bool isDeltaActive() const noexcept { return deltaActive.load(std::memory_order_relaxed); }

        void setCloudEndpoint(const juce::String& url) { cloudEndpoint = url; }

    private:
        void run() override;

        juce::AudioProcessorValueTreeState& apvts;
        juce::String cloudEndpoint { "https://europe-west1-jd-mix-ai-engine.cloudfunctions.net/analyzeMix" };

        std::atomic<AiStatus> currentStatus { AiStatus::Idle };
        juce::String statusMessage { "Ready" };
        std::atomic<float> lastConfidenceScore { 0.95f };

        std::atomic<bool> slotBActive { false };
        std::atomic<bool> deltaActive { false };

        analysis::MirAcousticSnapshot pendingSnapshot;
        juce::String pendingPrompt;
        float pendingIntensity { 100.0f };

        // Slot A and Slot B parameter stores
        std::map<juce::String, float> slotAParams;
        std::map<juce::String, float> slotBParams;

        GeminiPlanResponse parseJsonResponse(const juce::String& jsonText);
        void validateAndClampPlan(GeminiPlanResponse& plan);
        void applyPlanToApvts(const GeminiPlanResponse& plan, float intensity);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeminiClient)
    };
}
