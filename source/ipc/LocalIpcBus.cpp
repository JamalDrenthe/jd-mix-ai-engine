#include "LocalIpcBus.h"
#include <chrono>
#include <cstring>

#if JUCE_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace jdmix::ipc
{
    static uint64_t getCurrentTimeMs()
    {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    LocalIpcBus::LocalIpcBus(const juce::String& busName)
        : sharedMemName(busName)
    {
        initializeSharedMemory();
    }

    LocalIpcBus::~LocalIpcBus()
    {
        if (currentSlotIndex >= 0)
        {
            unregisterInstance(currentUuid);
        }
        cleanupSharedMemory();
    }

    void LocalIpcBus::initializeSharedMemory()
    {
#if JUCE_WINDOWS
        std::wstring wName = sharedMemName.toWideCharPointer();
        HANDLE hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                         sizeof(SharedMemoryLayout), wName.c_str());
        if (hMap != nullptr)
        {
            void* pBuf = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemoryLayout));
            if (pBuf != nullptr)
            {
                sharedLayout = static_cast<SharedMemoryLayout*>(pBuf);
                return;
            }
        }
#elif JUCE_LINUX || JUCE_MAC
        int fd = shm_open(("/" + sharedMemName).toRawUTF8(), O_CREAT | O_RDWR, 0666);
        if (fd >= 0)
        {
            ftruncate(fd, sizeof(SharedMemoryLayout));
            void* pBuf = mmap(nullptr, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            close(fd);
            if (pBuf != MAP_FAILED)
            {
                sharedLayout = static_cast<SharedMemoryLayout*>(pBuf);
                return;
            }
        }
#endif
        // Fallback to in-process memory map
        sharedLayout = &localProcessFallback;
    }

    void LocalIpcBus::cleanupSharedMemory()
    {
        // OS unmap handles if needed
        sharedLayout = nullptr;
    }

    bool LocalIpcBus::registerInstance(const juce::Uuid& uuid, const juce::String& name, int channelIndex)
    {
        if (sharedLayout == nullptr) return false;

        currentUuid = uuid;
        uint64_t now = getCurrentTimeMs();

        // Find available or expired slot
        for (int i = 0; i < maxIpcInstances; ++i)
        {
            auto& slot = sharedLayout->slots[i];
            if (!slot.isActive || (now - slot.lastHeartbeatMs > 5000))
            {
                currentSlotIndex = i;
                slot.isActive = true;
                slot.channelIndex = channelIndex;
                slot.lastHeartbeatMs = now;
                std::strncpy(slot.uuid, uuid.toString().toRawUTF8(), sizeof(slot.uuid) - 1);
                std::strncpy(slot.channelName, name.toRawUTF8(), sizeof(slot.channelName) - 1);
                return true;
            }
        }
        return false;
    }

    void LocalIpcBus::unregisterInstance(const juce::Uuid& uuid)
    {
        if (sharedLayout == nullptr) return;

        for (int i = 0; i < maxIpcInstances; ++i)
        {
            auto& slot = sharedLayout->slots[i];
            if (slot.isActive && std::strcmp(slot.uuid, uuid.toString().toRawUTF8()) == 0)
            {
                slot.isActive = false;
                slot.rmsLevel = 0.0f;
                slot.gainReduction = 0.0f;
                break;
            }
        }
        currentSlotIndex = -1;
    }

    void LocalIpcBus::updateTelemetry(const juce::Uuid& uuid,
                                     float rms,
                                     float gainReduction,
                                     float lufsShortTerm,
                                     const float* spectrum64)
    {
        if (sharedLayout == nullptr) return;

        int idx = currentSlotIndex;
        if (idx < 0 || idx >= maxIpcInstances ||
            std::strcmp(sharedLayout->slots[idx].uuid, uuid.toString().toRawUTF8()) != 0)
        {
            for (int i = 0; i < maxIpcInstances; ++i)
            {
                if (sharedLayout->slots[i].isActive &&
                    std::strcmp(sharedLayout->slots[i].uuid, uuid.toString().toRawUTF8()) == 0)
                {
                    idx = i;
                    currentSlotIndex = i;
                    break;
                }
            }
        }

        if (idx >= 0 && idx < maxIpcInstances)
        {
            auto& slot = sharedLayout->slots[idx];
            slot.rmsLevel = rms;
            slot.gainReduction = gainReduction;
            slot.lufsShortTerm = lufsShortTerm;
            slot.lastHeartbeatMs = getCurrentTimeMs();

            if (spectrum64 != nullptr)
            {
                std::memcpy(slot.spectrum64, spectrum64, sizeof(float) * ipcSpectrumBands);
            }
            sharedLayout->sequenceNumber.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void LocalIpcBus::setTrackRole(const juce::Uuid& uuid, int role)
    {
        if (sharedLayout == nullptr) return;
        for (int i = 0; i < maxIpcInstances; ++i)
        {
            auto& slot = sharedLayout->slots[i];
            if (slot.isActive && std::strcmp(slot.uuid, uuid.toString().toRawUTF8()) == 0)
            {
                slot.trackRole = role;
                break;
            }
        }
    }

    int LocalIpcBus::getActiveInstanceCount() const
    {
        if (sharedLayout == nullptr) return 1;
        uint64_t now = getCurrentTimeMs();
        int count = 0;
        for (int i = 0; i < maxIpcInstances; ++i)
        {
            if (sharedLayout->slots[i].isActive && (now - sharedLayout->slots[i].lastHeartbeatMs <= 3000))
                count++;
        }
        return juce::jmax(1, count);
    }

    std::vector<InstanceTelemetry> LocalIpcBus::getAllActiveInstances() const
    {
        std::vector<InstanceTelemetry> result;
        if (sharedLayout == nullptr) return result;
        uint64_t now = getCurrentTimeMs();
        for (int i = 0; i < maxIpcInstances; ++i)
        {
            const auto& slot = sharedLayout->slots[i];
            if (slot.isActive && (now - slot.lastHeartbeatMs <= 3000))
            {
                result.push_back(slot);
            }
        }
        return result;
    }

    bool LocalIpcBus::getMaskingSpectrum(int targetRole, std::array<float, ipcSpectrumBands>& dest) const
    {
        if (sharedLayout == nullptr) return false;
        uint64_t now = getCurrentTimeMs();
        for (int i = 0; i < maxIpcInstances; ++i)
        {
            const auto& slot = sharedLayout->slots[i];
            if (slot.isActive && slot.trackRole == targetRole && (now - slot.lastHeartbeatMs <= 3000))
            {
                std::memcpy(dest.data(), slot.spectrum64, sizeof(float) * ipcSpectrumBands);
                return true;
            }
        }
        return false;
    }
}
