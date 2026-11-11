#include "View3ChromaticOrb.h"
#include <cmath>

namespace jdmix::gui
{
    View3ChromaticOrb::View3ChromaticOrb(juce::AudioProcessorValueTreeState& state, ai::GeminiClient& client)
        : apvts(state),
          aiClient(client)
    {
        modelSelector.addItemList({ "Vocal Model: Honeyed Soul", "Vocal Model: Modern Trapsoul",
                                    "Vocal Model: Crisp Commercial", "Vocal Model: Vintage Tube Warmth" }, 1);
        modelSelector.setSelectedId(1);
        addAndMakeVisible(modelSelector);

        executeAiButton.setColour(juce::TextButton::buttonColourId, ThemeColors::PurpleNeon);
        executeAiButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        executeAiButton.onClick = [this]()
        {
            if (onExecuteAiPlan)
                onExecuteAiPlan();
        };
        addAndMakeVisible(executeAiButton);

        rangeMatchingToggle.setColour(juce::ToggleButton::textColourId, ThemeColors::TextSecondary);
        rangeMatchingToggle.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(rangeMatchingToggle);
    }

    View3ChromaticOrb::~View3ChromaticOrb() = default;

    void View3ChromaticOrb::setAudioTelemetry(float rms, float formantShift)
    {
        currentRms = rms;
        currentFormant = formantShift;
        animationPhase += 0.05f;
        if (animationPhase > 1000.0f) animationPhase = 0.0f;
        repaint();
    }

    void View3ChromaticOrb::mouseDown(const juce::MouseEvent& e)
    {
        if (cachedOrbArea.contains(e.position))
        {
            shockwavePos = e.position;
            shockwaveRadius = 6.0f;
            shockwaveAlpha = 1.0f;
            repaint();
        }
    }

    void View3ChromaticOrb::mouseMove(const juce::MouseEvent& e)
    {
        mousePos = e.position;
        if (cachedOrbArea.contains(e.position))
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        else if (cachedTimelineArea.contains(e.position))
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
        else
            setMouseCursor(juce::MouseCursor::NormalCursor);

        repaint();
    }

    void View3ChromaticOrb::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
    {
        if (cachedTimelineArea.contains(e.position))
        {
            timelineZoom = juce::jlimit(0.5f, 3.0f, timelineZoom + wheel.deltaY * 0.15f);
            timelineOffset = juce::jlimit(-120.0f, 120.0f, timelineOffset + wheel.deltaX * 25.0f);
            repaint();
        }
        else
        {
            Component::mouseWheelMove(e, wheel);
        }
    }

    void View3ChromaticOrb::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat().reduced(6.0f);

        // Dark obsidian chassis with luxury micro-border sheen
        g.setColour(ThemeColors::ObsidianBg);
        g.fillRoundedRectangle(bounds, 12.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(bounds, 12.0f, 1.0f);

        // Draw JD Logo Mark in View 3 Header
        JDMixLookAndFeel::drawJdLogoMark(g, juce::Rectangle<float>(bounds.getX() + 15.0f, bounds.getY() + 12.0f, 22.0f, 22.0f), ThemeColors::CyanNeon);

        // Header Title
        g.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
        g.setColour(ThemeColors::TextPrimary);
        g.drawText("AI RESYNTH & CHROMATIC LIQUID ORB", bounds.getX() + 44.0f, bounds.getY() + 12.0f, 320.0f, 22.0f, juce::Justification::left);

        // Status badge
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        if (aiClient.getStatus() == ai::AiStatus::Analyzing || aiClient.getStatus() == ai::AiStatus::Applying)
        {
            g.setColour(ThemeColors::CyanNeon);
            g.drawText(aiClient.getStatusMessage(), bounds.getX() + 15.0f, bounds.getY() + 34.0f, 400.0f, 16.0f, juce::Justification::left);
        }
        else
        {
            g.setColour(ThemeColors::SafeEmerald);
            g.drawText(aiClient.getStatusMessage(), bounds.getX() + 15.0f, bounds.getY() + 34.0f, 400.0f, 16.0f, juce::Justification::left);
        }

        // Center Area: Fluid Orb
        auto centerArea = bounds.reduced(10.0f);
        centerArea.removeFromTop(44.0f);
        auto timelineArea = centerArea.removeFromBottom(75.0f);
        cachedTimelineArea = timelineArea;

        auto orbArea = centerArea;
        cachedOrbArea = orbArea;
        drawLiquidOrb(g, orbArea);

        // Timeline Area
        timelineArea.removeFromBottom(42.0f);
        drawWaveformTimeline(g, timelineArea);
    }

    void View3ChromaticOrb::drawLiquidOrb(juce::Graphics& g, juce::Rectangle<float> orbArea)
    {
        auto centre = orbArea.getCentre();
        float radius = orbArea.getWidth() * 0.44f;

        // Dark sunken lens with deep drop shadow
        g.setColour(juce::Colour(0xff080b12));
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        // Mouse hover directional magnetic attraction
        float mouseAngle = 0.0f;
        float mouseDist = 999.0f;
        if (cachedOrbArea.contains(mousePos))
        {
            float dx = mousePos.x - centre.x;
            float dy = mousePos.y - centre.y;
            mouseDist = std::sqrt(dx * dx + dy * dy);
            mouseAngle = std::atan2(dy, dx);
        }
        float pullStrength = (mouseDist < radius * 1.3f) ? (1.0f - mouseDist / (radius * 1.3f)) * 14.0f : 0.0f;

        // 4 Concentric Iridescent Rings
        bool isTransforming = (aiClient.getStatus() == ai::AiStatus::Analyzing ||
                              aiClient.getStatus() == ai::AiStatus::Applying);
        float speed = isTransforming ? 3.0f : 1.0f;
        float dynAmp = 6.0f + currentRms * 18.0f + (isTransforming ? 10.0f : 0.0f);

        juce::Colour ringColours[4] = {
            ThemeColors::CyanNeon,
            ThemeColors::LimeNeon,
            ThemeColors::PurpleNeon,
            ThemeColors::AmberGold
        };

        for (int r = 0; r < 4; ++r)
        {
            float baseRad = radius * (0.35f + r * 0.16f);
            juce::Path ringPath;

            constexpr int points = 64;
            for (int p = 0; p <= points; ++p)
            {
                float angle = (p / static_cast<float>(points)) * juce::MathConstants<float>::twoPi;
                float mod = std::sin(angle * (5.0f + r * 2.0f) + animationPhase * speed + r * 1.3f) * dynAmp
                          + std::cos(angle * 3.0f - animationPhase * speed * 0.7f) * (dynAmp * 0.5f);

                // Add magnetic ferrofluid pull toward mouse
                float angleDiff = std::cos(angle - mouseAngle);
                float magneticPull = (angleDiff > 0.0f) ? (angleDiff * angleDiff * pullStrength) : 0.0f;

                float curR = baseRad + mod + magneticPull;
                float x = centre.x + std::cos(angle) * curR;
                float y = centre.y + std::sin(angle) * curR;

                if (p == 0)
                    ringPath.startNewSubPath(x, y);
                else
                    ringPath.lineTo(x, y);
            }
            ringPath.closeSubPath();

            // Glow + sharp stroke
            g.setColour(ringColours[r].withAlpha(0.35f));
            g.strokePath(ringPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved));
            g.setColour(ringColours[r]);
            g.strokePath(ringPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
        }

        // Lens specular sheen
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.fillEllipse(centre.x - radius * 0.8f, centre.y - radius * 0.85f, radius * 1.6f, radius * 0.8f);

        // Pulsating Center JD Brand Core
        juce::Colour coreColour = isTransforming ? ThemeColors::PurpleNeon : ThemeColors::CyanNeon;
        auto coreBounds = juce::Rectangle<float>(centre.x - 22.0f, centre.y - 15.0f, 44.0f, 30.0f);
        float pulseAlpha = 0.55f + 0.35f * std::sin(animationPhase * (isTransforming ? 4.0f : 1.5f));
        JDMixLookAndFeel::drawJdLogoMark(g, coreBounds, coreColour.withAlpha(pulseAlpha));

        // 3D Chromatic Shockwave Ripple Ring on Click
        if (shockwaveAlpha > 0.02f)
        {
            // Red / Cyan chromatic aberration split
            g.setColour(ThemeColors::CyanNeon.withAlpha(shockwaveAlpha * 0.75f));
            g.drawEllipse(shockwavePos.x - shockwaveRadius - 2.0f,
                          shockwavePos.y - shockwaveRadius * 0.75f,
                          shockwaveRadius * 2.0f, shockwaveRadius * 1.5f, 2.5f);

            g.setColour(ThemeColors::PurpleNeon.withAlpha(shockwaveAlpha * 0.75f));
            g.drawEllipse(shockwavePos.x - shockwaveRadius + 2.0f,
                          shockwavePos.y - shockwaveRadius * 0.75f,
                          shockwaveRadius * 2.0f, shockwaveRadius * 1.5f, 2.0f);

            shockwaveRadius += 3.8f;
            shockwaveAlpha -= 0.045f;
            repaint();
        }
    }

    void View3ChromaticOrb::drawWaveformTimeline(juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setColour(ThemeColors::GraphitePanel);
        g.fillRoundedRectangle(area, 6.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(area, 6.0f, 1.0f);

        // Timeline waveform with interactive zoom and offset
        int numBars = static_cast<int>(50 * timelineZoom);
        numBars = juce::jlimit(20, 120, numBars);
        float barW = area.getWidth() / numBars;

        for (int i = 0; i < numBars; ++i)
        {
            float x = area.getX() + i * barW + timelineOffset * 0.2f;
            if (x < area.getX() || x > area.getRight() - barW)
                continue;

            float h = 6.0f + std::sin(i * 0.4f + animationPhase) * 10.0f + std::cos(i * 0.8f) * 6.0f;
            h = juce::jlimit(4.0f, area.getHeight() - 6.0f, h);

            int sliceStart = static_cast<int>(15 * timelineZoom);
            int sliceEnd = static_cast<int>(35 * timelineZoom);

            if (i >= sliceStart && i <= sliceEnd) // Highlighted active AI slice
            {
                g.setColour(ThemeColors::PurpleNeon.withAlpha(0.85f));
            }
            else
            {
                g.setColour(ThemeColors::TextSecondary.withAlpha(0.35f));
            }

            g.fillRect(x + 1.0f, area.getCentreY() - h * 0.5f, barW - 2.0f, h);
        }

        // Hover Playhead Indicator
        if (cachedTimelineArea.contains(mousePos))
        {
            g.setColour(ThemeColors::CyanNeon);
            g.drawLine(mousePos.x, area.getY() + 2.0f, mousePos.x, area.getBottom() - 2.0f, 1.5f);
        }

        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("00:30 (SCROLL TO ZOOM/SCRUB)", area.getX() + 6.0f, area.getY() + 2.0f, 150.0f, 10.0f, juce::Justification::left);
        g.drawText("01:00", area.getRight() - 34.0f, area.getY() + 2.0f, 30.0f, 10.0f, juce::Justification::right);
    }

    void View3ChromaticOrb::resized()
    {
        auto bounds = getLocalBounds().reduced(12);

        modelSelector.setBounds(bounds.getRight() - 210, bounds.getY() + 8, 200, 26);

        auto botArea = bounds.removeFromBottom(85);
        botArea.removeFromTop(36); // Waveform space

        executeAiButton.setBounds(botArea.getX() + 20, botArea.getY() + 6, botArea.getWidth() - 200, 36);
        rangeMatchingToggle.setBounds(botArea.getRight() - 170, botArea.getY() + 10, 160, 26);
    }
}
