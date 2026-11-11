#include "View5StemMasterRadar.h"
#include "../../core/ParamRegistry.h"
#include <cmath>

namespace jdmix::gui
{
    View5StemMasterRadar::View5StemMasterRadar(juce::AudioProcessorValueTreeState& state, ipc::LocalIpcBus& ipc)
        : apvts(state),
          ipcBus(ipc)
    {
        sectorEnergiesDb.fill(-80.0f);

        auto setupSlider = [this](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::LinearHorizontal);
            s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 55, 18);
            s.setColour(juce::Slider::trackColourId, ThemeColors::PurpleNeon.withAlpha(0.6f));
            s.setColour(juce::Slider::thumbColourId, ThemeColors::PurpleNeon);
            addAndMakeVisible(s);
        };

        setupSlider(targetLufsSlider);
        setupSlider(vocalPocketSlider);
        setupSlider(monoBassSlider);

        targetLufsAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::masterTargetLufs, targetLufsSlider);
        vocalPocketAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::masterVocalPocket, vocalPocketSlider);
        monoBassAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::masterMonoBassFreq, monoBassSlider);

        // 40-Slot Memory Matrix
        for (int i = 0; i < 40; ++i)
        {
            memoryButtons[i].setButtonText(juce::String(i + 1));
            memoryButtons[i].setColour(juce::TextButton::buttonColourId, ThemeColors::GraphitePanel);
            memoryButtons[i].onClick = [this, i]()
            {
                activeMemorySlot = i;
                for (int b = 0; b < 40; ++b)
                {
                    memoryButtons[b].setColour(juce::TextButton::buttonColourId,
                        (b == activeMemorySlot) ? ThemeColors::PurpleNeon.withAlpha(0.4f) : ThemeColors::GraphitePanel);
                }
                repaint();
            };
            addAndMakeVisible(memoryButtons[i]);
        }
        memoryButtons[0].setColour(juce::TextButton::buttonColourId, ThemeColors::PurpleNeon.withAlpha(0.4f));
    }

    View5StemMasterRadar::~View5StemMasterRadar() = default;

    void View5StemMasterRadar::updateRadarData()
    {
        // Pull telemetry from IPC to populate 48 sectors
        auto instances = ipcBus.getAllActiveInstances();
        for (size_t i = 0; i < instances.size() && i < 4; ++i)
        {
            int sectorOffset = static_cast<int>(i * 12);
            float levelDb = juce::Decibels::gainToDecibels(instances[i].rmsLevel, -80.0f);

            for (int s = 0; s < 12; ++s)
            {
                int sec = (sectorOffset + s) % 48;
                sectorEnergiesDb[sec] = sectorEnergiesDb[sec] * 0.85f + levelDb * 0.15f;
            }
        }
        repaint();
    }

    void View5StemMasterRadar::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat().reduced(6.0f);

        // Brushed Black Anodized Chassis
        g.setColour(ThemeColors::ObsidianBg);
        g.fillRoundedRectangle(bounds, 12.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(bounds, 12.0f, 1.0f);

        // Top labels for horizontal sliders
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("TARGET LUFS (-14 TARGET)", bounds.getX() + 15.0f, bounds.getY() + 8.0f, 180.0f, 14.0f, juce::Justification::left);
        g.drawText("VOCAL POCKET (-5.0 LUFS)", bounds.getX() + bounds.getWidth() * 0.35f, bounds.getY() + 8.0f, 180.0f, 14.0f, juce::Justification::left);
        g.drawText("MONO-BASS (90 HZ)", bounds.getX() + bounds.getWidth() * 0.70f, bounds.getY() + 8.0f, 180.0f, 14.0f, juce::Justification::left);

        // Radar Canvas in Center
        auto radarArea = juce::Rectangle<float>(bounds.getCentreX() - 110.0f, bounds.getY() + 60.0f, 220.0f, 220.0f);
        drawPolarRadar(g, radarArea);

        // Memory Matrix Header
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.setColour(ThemeColors::TextPrimary);
        g.drawText("40-SLOT HARDWARE MEMORY MATRIX", bounds.getX() + 15.0f, bounds.getBottom() - 95.0f, 250.0f, 16.0f, juce::Justification::left);
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.setColour(ThemeColors::PurpleNeon);
        g.drawText("ACTIVE SLOT: " + juce::String(activeMemorySlot + 1) + " | 4-STEM MASTER READY",
                   bounds.getRight() - 250.0f, bounds.getBottom() - 95.0f, 235.0f, 16.0f, juce::Justification::right);
    }

    void View5StemMasterRadar::drawPolarRadar(juce::Graphics& g, juce::Rectangle<float> area)
    {
        auto centre = area.getCentre();
        float radius = area.getWidth() * 0.48f;

        // Dark radar circle
        g.setColour(juce::Colour(0xff0d0f17));
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        // Concentric decibel rings (0, -12, -24, -40, -60, -80 dB)
        const float dbLevels[] = { 0.0f, -12.0f, -24.0f, -40.0f, -60.0f };
        for (float db : dbLevels)
        {
            float normR = juce::jmap(db, -80.0f, 0.0f, 0.1f, 1.0f) * radius;
            g.setColour(juce::Colours::white.withAlpha(0.08f));
            g.drawEllipse(centre.x - normR, centre.y - normR, normR * 2.0f, normR * 2.0f, 1.0f);
        }

        // 48 Sectors
        for (int i = 0; i < 48; ++i)
        {
            float angle = (static_cast<float>(i) / 48.0f) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float valDb = sectorEnergiesDb[i];
            float normR = juce::jmap(valDb, -80.0f, 0.0f, 0.15f, 0.95f) * radius;
            normR = juce::jlimit(radius * 0.15f, radius * 0.95f, normR);

            float x = centre.x + std::cos(angle) * normR;
            float y = centre.y + std::sin(angle) * normR;

            juce::Path seg;
            seg.startNewSubPath(centre.x, centre.y);
            seg.lineTo(x, y);

            // Lead vocal top sectors highlight
            if (i >= 46 || i <= 2)
            {
                g.setColour(ThemeColors::CyanNeon.withAlpha(0.6f));
                g.strokePath(seg, juce::PathStrokeType(2.5f));
            }
            else
            {
                g.setColour(ThemeColors::PurpleNeon.withAlpha(0.45f));
                g.strokePath(seg, juce::PathStrokeType(1.5f));
            }
        }

        // Sector Numbers on perimeter (1, 12, 24, 36, 48)
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("1", centre.x - 10.0f, centre.y - radius - 1.0f, 20.0f, 12.0f, juce::Justification::centred);
        g.drawText("12", centre.x + radius - 12.0f, centre.y - 6.0f, 20.0f, 12.0f, juce::Justification::centred);
        g.drawText("24", centre.x - 10.0f, centre.y + radius - 12.0f, 20.0f, 12.0f, juce::Justification::centred);
        g.drawText("36", centre.x - radius - 6.0f, centre.y - 6.0f, 20.0f, 12.0f, juce::Justification::centred);
    }

    void View5StemMasterRadar::resized()
    {
        auto bounds = getLocalBounds().reduced(12);

        // Top Horizontal Sliders
        int sW = (bounds.getWidth() - 30) / 3;
        targetLufsSlider.setBounds(bounds.getX(), bounds.getY() + 24, sW, 24);
        vocalPocketSlider.setBounds(bounds.getX() + sW + 15, bounds.getY() + 24, sW, 24);
        monoBassSlider.setBounds(bounds.getX() + (sW + 15) * 2, bounds.getY() + 24, sW, 24);

        // Bottom 40 Memory buttons (4 rows x 10 cols)
        auto botArea = bounds.removeFromBottom(75);
        int btnW = botArea.getWidth() / 10;
        int btnH = 16;

        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 10; ++col)
            {
                int idx = row * 10 + col;
                memoryButtons[idx].setBounds(botArea.getX() + col * btnW + 1,
                                            botArea.getY() + row * (btnH + 2),
                                            btnW - 2, btnH);
            }
        }
    }
}
