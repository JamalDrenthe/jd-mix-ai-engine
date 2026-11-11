#include "View1EqCleanDecay.h"
#include "../../core/ParamRegistry.h"
#include <cmath>

namespace jdmix::gui
{
    View1EqCleanDecay::View1EqCleanDecay(juce::AudioProcessorValueTreeState& state)
        : apvts(state)
    {
        currentSpectrum.fill(-100.0f);
        currentGhostTrail.fill(-100.0f);
        maskSpectrum.fill(-100.0f);

        auto setupSlider = [this](juce::Slider& s, const juce::Colour& colour)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            s.setColour(juce::Slider::rotarySliderFillColourId, colour);
            s.setScrollWheelEnabled(true);
            s.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
            addAndMakeVisible(s);
        };

        setupSlider(brightnessSlider,  ThemeColors::CyanNeon);
        setupSlider(characterSlider,   ThemeColors::CyanNeon);
        setupSlider(distanceSlider,    ThemeColors::CyanNeon);
        setupSlider(decayRateSlider,   ThemeColors::CyanNeon);
        setupSlider(stereoWidthSlider, ThemeColors::CyanNeon);
        setupSlider(mixSlider,         ThemeColors::AmberGold);

        // Hero Arch Dial
        heroArchDial.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        heroArchDial.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        heroArchDial.setColour(juce::Slider::rotarySliderFillColourId, ThemeColors::CyanNeon);
        heroArchDial.setScrollWheelEnabled(true);
        heroArchDial.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        addAndMakeVisible(heroArchDial);

        brightnessAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::eqAddAirGain, brightnessSlider);
        characterAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::toneSatDrive, characterSlider);
        distanceAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::sendPlateSend, distanceSlider);
        decayRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::sendShortRoom, decayRateSlider);
        stereoWidthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::dynCompParBlend, stereoWidthSlider);
        mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::toneSatMix, mixSlider);
        heroArchAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::gainClipTrim, heroArchDial);
    }

    View1EqCleanDecay::~View1EqCleanDecay() = default;

    void View1EqCleanDecay::updateSpectrum(const std::array<float, analysis::SpectrumAnalyzer::numBands64>& spectrum,
                                          const std::array<float, analysis::SpectrumAnalyzer::numBands64>* masking)
    {
        currentSpectrum = spectrum;
        for (size_t i = 0; i < currentSpectrum.size(); ++i)
        {
            if (currentSpectrum[i] > currentGhostTrail[i])
                currentGhostTrail[i] = currentSpectrum[i];
            else
                currentGhostTrail[i] = currentGhostTrail[i] * 0.94f + currentSpectrum[i] * 0.06f;
        }

        if (masking != nullptr)
        {
            maskSpectrum = *masking;
            hasMasking = true;
        }
        else
        {
            hasMasking = false;
        }

        repaint();
    }

    void View1EqCleanDecay::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        // 1. Top Ribbon Background
        auto topArea = bounds.removeFromTop(120.0f);
        g.setColour(ThemeColors::GraphitePanel);
        g.fillRoundedRectangle(topArea.reduced(4.0f), 8.0f);

        // Labels for top knobs
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(ThemeColors::TextSecondary);

        auto dialW = topArea.getWidth() / 7.0f;
        g.drawText("BRIGHTNESS\nDARK  BRIGHT", topArea.getX(), topArea.getBottom() - 32.0f, dialW, 28.0f, juce::Justification::centred);
        g.drawText("CHARACTER\nCLEAN  CHORUS", topArea.getX() + dialW, topArea.getBottom() - 32.0f, dialW, 28.0f, juce::Justification::centred);
        g.drawText("DISTANCE\nCLOSE  FAR", topArea.getX() + dialW * 2, topArea.getBottom() - 32.0f, dialW, 28.0f, juce::Justification::centred);

        // Center Hero Readout
        g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
        g.setColour(ThemeColors::TextPrimary);
        float trimVal = heroArchDial.getValue();
        g.drawText(juce::String(trimVal, 1) + " dB", topArea.getX() + dialW * 3, topArea.getY() + 45.0f, dialW, 20.0f, juce::Justification::centred);
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(ThemeColors::AmberGold);
        g.drawText("- SPACE / TARGET GAIN -", topArea.getX() + dialW * 3, topArea.getBottom() - 25.0f, dialW, 20.0f, juce::Justification::centred);

        g.setColour(ThemeColors::TextSecondary);
        g.drawText("DECAY RATE\n50%  200%", topArea.getX() + dialW * 4, topArea.getBottom() - 32.0f, dialW, 28.0f, juce::Justification::centred);
        g.drawText("STEREO WIDTH\nMONO  SPACIOUS", topArea.getX() + dialW * 5, topArea.getBottom() - 32.0f, dialW, 28.0f, juce::Justification::centred);
        g.drawText("🔒 MIX\n0%  100%", topArea.getX() + dialW * 6, topArea.getBottom() - 32.0f, dialW, 28.0f, juce::Justification::centred);

        // 2. Spectrum Canvas
        auto specArea = bounds.reduced(4.0f);
        g.setColour(ThemeColors::ObsidianBg);
        g.fillRoundedRectangle(specArea, 8.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(specArea, 8.0f, 1.0f);

        drawSpectrumGrid(g, specArea);
        drawSmokeFft(g, specArea);
        drawCyanDynamicCurve(g, specArea);
        drawGoldEqCurve(g, specArea);
    }

    void View1EqCleanDecay::drawSpectrumGrid(juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.setColour(juce::Colours::white.withAlpha(0.15f));

        const float freqs[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
        const char* labels[] = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };

        for (int i = 0; i < 10; ++i)
        {
            float normX = std::log10(freqs[i] / 20.0f) / std::log10(1000.0f);
            float x = area.getX() + normX * area.getWidth();
            g.drawVerticalLine(static_cast<int>(x), area.getY(), area.getBottom() - 16.0f);
            g.drawText(labels[i], x - 15.0f, area.getBottom() - 16.0f, 30.0f, 14.0f, juce::Justification::centred);
        }

        // Horizontal dB lines
        const float dbs[] = { 12.0f, 6.0f, 0.0f, -6.0f, -12.0f, -18.0f };
        for (float db : dbs)
        {
            float normY = juce::jmap(db, 18.0f, -24.0f, 0.0f, 1.0f);
            float y = area.getY() + normY * area.getHeight();
            g.drawHorizontalLine(static_cast<int>(y), area.getX(), area.getRight());
            g.drawText(juce::String(static_cast<int>(db)), area.getRight() - 25.0f, y - 8.0f, 22.0f, 14.0f, juce::Justification::right);
        }
    }

    void View1EqCleanDecay::drawSmokeFft(juce::Graphics& g, juce::Rectangle<float> area)
    {
        juce::Path ghostPath;
        juce::Path livePath;

        const int numBands = analysis::SpectrumAnalyzer::numBands64;
        for (int i = 0; i < numBands; ++i)
        {
            float x = area.getX() + (static_cast<float>(i) / (numBands - 1)) * area.getWidth();
            float liveY = area.getY() + juce::jmap(currentSpectrum[i], 0.0f, -80.0f, 0.0f, area.getHeight());
            float ghostY = area.getY() + juce::jmap(currentGhostTrail[i], 0.0f, -80.0f, 0.0f, area.getHeight());

            liveY = juce::jlimit(area.getY(), area.getBottom(), liveY);
            ghostY = juce::jlimit(area.getY(), area.getBottom(), ghostY);

            if (i == 0)
            {
                livePath.startNewSubPath(x, liveY);
                ghostPath.startNewSubPath(x, ghostY);
            }
            else
            {
                livePath.lineTo(x, liveY);
                ghostPath.lineTo(x, ghostY);
            }
        }

        // Layered smoke glow
        g.setColour(ThemeColors::AmberGold.withAlpha(0.12f));
        g.strokePath(ghostPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved));

        g.setColour(juce::Colours::white.withAlpha(0.40f));
        g.strokePath(livePath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved));

        // External Masking collision alert
        if (hasMasking)
        {
            juce::Path maskPath;
            for (int i = 0; i < numBands; ++i)
            {
                float x = area.getX() + (static_cast<float>(i) / (numBands - 1)) * area.getWidth();
                float myDb = currentSpectrum[i];
                float theirDb = maskSpectrum[i];

                if (myDb > -30.0f && theirDb > -30.0f && std::abs(myDb - theirDb) < 4.0f)
                {
                    // Collision glow at overlapping frequencies
                    g.setColour(ThemeColors::WarningRed.withAlpha(0.35f));
                    g.fillEllipse(x - 8.0f, area.getBottom() - 30.0f, 16.0f, 16.0f);
                }
            }
        }
    }

    void View1EqCleanDecay::drawCyanDynamicCurve(juce::Graphics& g, juce::Rectangle<float> area)
    {
        juce::Path cyanPath;
        cyanPath.startNewSubPath(area.getX(), area.getY() + area.getHeight() * 0.35f);
        cyanPath.quadraticTo(area.getX() + area.getWidth() * 0.3f, area.getY() + area.getHeight() * 0.25f,
                             area.getX() + area.getWidth() * 0.5f, area.getY() + area.getHeight() * 0.40f);
        cyanPath.quadraticTo(area.getX() + area.getWidth() * 0.8f, area.getY() + area.getHeight() * 0.20f,
                             area.getRight(), area.getY() + area.getHeight() * 0.35f);

        g.setColour(ThemeColors::CyanNeon.withAlpha(0.30f));
        g.strokePath(cyanPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved));
        g.setColour(ThemeColors::CyanNeon);
        g.strokePath(cyanPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
    }

    void View1EqCleanDecay::drawGoldEqCurve(juce::Graphics& g, juce::Rectangle<float> area)
    {
        juce::Path goldPath;
        float hpfY = area.getBottom();
        float midY = area.getY() + area.getHeight() * 0.5f;

        goldPath.startNewSubPath(area.getX(), hpfY);
        goldPath.quadraticTo(area.getX() + area.getWidth() * 0.15f, hpfY,
                             area.getX() + area.getWidth() * 0.20f, midY);
        goldPath.quadraticTo(area.getX() + area.getWidth() * 0.35f, midY + 25.0f,
                             area.getX() + area.getWidth() * 0.50f, midY);
        goldPath.quadraticTo(area.getX() + area.getWidth() * 0.75f, midY - 20.0f,
                             area.getRight(), midY - 15.0f);

        // Translucent golden fill under EQ curve
        juce::Path fillPath = goldPath;
        fillPath.lineTo(area.getRight(), area.getBottom());
        fillPath.lineTo(area.getX(), area.getBottom());
        fillPath.closeSubPath();

        g.setColour(ThemeColors::AmberGold.withAlpha(0.08f));
        g.fillPath(fillPath);

        g.setColour(ThemeColors::AmberGold);
        g.strokePath(goldPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved));
    }

    void View1EqCleanDecay::resized()
    {
        auto bounds = getLocalBounds();
        auto topArea = bounds.removeFromTop(120).reduced(8);
        int dialW = topArea.getWidth() / 7;

        brightnessSlider.setBounds(topArea.getX(), topArea.getY() + 10, dialW, 55);
        characterSlider.setBounds(topArea.getX() + dialW, topArea.getY() + 10, dialW, 55);
        distanceSlider.setBounds(topArea.getX() + dialW * 2, topArea.getY() + 10, dialW, 55);

        // Center Hero Dial
        heroArchDial.setBounds(topArea.getX() + dialW * 3 - 5, topArea.getY() - 5, dialW + 10, 70);

        decayRateSlider.setBounds(topArea.getX() + dialW * 4, topArea.getY() + 10, dialW, 55);
        stereoWidthSlider.setBounds(topArea.getX() + dialW * 5, topArea.getY() + 10, dialW, 55);
        mixSlider.setBounds(topArea.getX() + dialW * 6, topArea.getY() + 10, dialW, 55);
    }
}
