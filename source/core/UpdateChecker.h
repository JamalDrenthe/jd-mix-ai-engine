#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <atomic>
#include <functional>

namespace jdmix::core
{
    /**
     * @brief Non-blocking background update checker for JD Mix AI Engine.
     * 
     * Queries the GitHub Releases API (https://api.github.com/repos/JamalDrenthe/jd-mix-ai-engine/releases/latest)
     * without blocking the audio thread or the GUI thread.
     */
    class UpdateChecker : private juce::Thread
    {
    public:
        UpdateChecker(juce::String currentVersionStr = "1.0.0")
            : juce::Thread("JDMixUpdateChecker"),
              currentVersion(currentVersionStr)
        {
        }

        ~UpdateChecker() override
        {
            stopThread(3000);
        }

        void checkForUpdatesAsync(std::function<void(bool isUpdateAvailable, juce::String latestVer, juce::String downloadUrl)> callback)
        {
            onCheckCompleted = std::move(callback);
            startThread(juce::Thread::Priority::low);
        }

        bool isUpdateAvailable() const noexcept { return updateAvailable.load(); }
        juce::String getLatestVersion() const { return latestVersion; }
        juce::String getDownloadUrl() const { return releaseUrl; }

    private:
        void run() override
        {
            // Wait 2 seconds after plugin startup before making network check to prioritize DAW audio initialization
            wait(2000);

            if (threadShouldExit())
                return;

            juce::String apiUrl = "https://api.github.com/repos/JamalDrenthe/jd-mix-ai-engine/releases/latest";
            juce::URL url(apiUrl);

            // GitHub API requires a User-Agent header
            juce::StringArray responseHeaders;
            int statusCode = 0;

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                               .withConnectionTimeoutMs(4000)
                               .withNumRedirectsToFollow(3)
                               .withExtraHeaders("User-Agent: JDMixAIEngine-Plugin-Client/" + currentVersion);

            std::unique_ptr<juce::InputStream> stream = url.createInputStream(options);

            if (stream == nullptr || threadShouldExit())
                return;

            juce::String jsonStr = stream->readEntireStreamAsString();
            auto parsed = juce::JSON::parse(jsonStr);

            if (!parsed.isObject())
                return;

            juce::String tagName = parsed["tag_name"].toString().trimCharactersAtStart("vV");
            juce::String htmlUrl = parsed["html_url"].toString();

            if (tagName.isNotEmpty())
            {
                bool isNewer = compareVersions(tagName, currentVersion) > 0;
                updateAvailable.store(isNewer);
                latestVersion = tagName;
                releaseUrl = htmlUrl.isNotEmpty() ? htmlUrl : "https://github.com/JamalDrenthe/jd-mix-ai-engine/releases";

                if (onCheckCompleted)
                {
                    juce::MessageManager::callAsync([this, isNewer, tag = latestVersion, dl = releaseUrl]()
                    {
                        if (onCheckCompleted)
                            onCheckCompleted(isNewer, tag, dl);
                    });
                }
            }
        }

        static int compareVersions(const juce::String& v1, const juce::String& v2)
        {
            auto tokens1 = juce::StringArray::fromTokens(v1, ".", "");
            auto tokens2 = juce::StringArray::fromTokens(v2, ".", "");

            int count = juce::jmax(tokens1.size(), tokens2.size());
            for (int i = 0; i < count; ++i)
            {
                int num1 = (i < tokens1.size()) ? tokens1[i].getIntValue() : 0;
                int num2 = (i < tokens2.size()) ? tokens2[i].getIntValue() : 0;

                if (num1 > num2) return 1;
                if (num1 < num2) return -1;
            }
            return 0;
        }

        const juce::String currentVersion;
        std::atomic<bool> updateAvailable { false };
        juce::String latestVersion { "1.0.0" };
        juce::String releaseUrl { "https://github.com/JamalDrenthe/jd-mix-ai-engine/releases" };
        std::function<void(bool, juce::String, juce::String)> onCheckCompleted;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UpdateChecker)
    };
}
