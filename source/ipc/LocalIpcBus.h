#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <string>
#include <vector>

namespace jdmix::ipc
{
    static constexpr int maxIpcInstances = 32;
    static constexpr int ipcSpectrumBands = 64;

    struct InstanceTelemetry
    {
        char uuid[40] { 0 };
        char channelName[64] { 0 };
        int channelIndex { 0 };
        int trackRole { 0 }; // Lead Vox, 808, Kick, Master, etc.
        uint32_t colorArgb { 0xff00e5ff };

        float rmsLevel { 0.0f };
        float peakLevel { 0.0f };
        float lufsShortTerm { -100.0f };
        float gainReduction { 0.0f };

        float spectrum64[ipcSpectrumBands] { 0.0f };

        uint64_t lastHeartbeatMs { 0 };
        bool isActive { false };
    };

    struct SharedMemoryLayout
    {
        std::atomic<uint32_t> version { 1 };
        std::atomic<uint32_t> sequenceNumber { 0 };
        InstanceTelemetry slots[maxIpcInstances];
    };

    class LocalIpcBus
    {
    public:
        explicit LocalIpcBus(const juce::String& busName = "JDMixAIEngine_SharedMem");
        ~LocalIpcBus();

        bool registerInstance(const juce::Uuid& uuid, const juce::String& name, int channelIndex);
        void unregisterInstance(const juce::Uuid& uuid);

        void updateTelemetry(const juce::Uuid& uuid,
                             float rms,
                             float gainReduction,
                             float lufsShortTerm = -14.0f,
                             const float* spectrum64 = nullptr);

        void setTrackRole(const juce::Uuid& uuid, int role);

        // Readers
        int getActiveInstanceCount() const;
        std::vector<InstanceTelemetry> getAllActiveInstances() const;
        bool getMaskingSpectrum(int targetRole, std::array<float, ipcSpectrumBands>& dest) const;

    private:
        juce::String sharedMemName;
        int currentSlotIndex { -1 };
        juce::Uuid currentUuid;

        SharedMemoryLayout* sharedLayout { nullptr };

        // Process-local fallback memory if OS shared memory is unavailable
        static inline SharedMemoryLayout localProcessFallback {};

        void initializeSharedMemory();
        void cleanupSharedMemory();
    };
}
