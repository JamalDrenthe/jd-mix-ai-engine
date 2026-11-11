#pragma once
#include <string>
#include <vector>

namespace jdmix::ai
{
    struct ParameterAdjustment
    {
        std::string paramId;
        float targetValue { 0.0f };
        std::string unit;
        std::string reason;
    };

    struct TrackAdjustment
    {
        std::string instanceId;
        std::string trackRole;
        std::string explanation;
        std::vector<ParameterAdjustment> parameters;
    };

    struct GeminiPlanResponse
    {
        std::string analysisSummary;
        float confidenceScore { 0.95f };
        float targetLufs { -14.0f };
        std::string detectedKey { "F#" };
        std::string detectedScale { "Minor" };
        std::vector<TrackAdjustment> trackAdjustments;
        bool isSuccess { false };
        std::string errorMessage;
    };
}
