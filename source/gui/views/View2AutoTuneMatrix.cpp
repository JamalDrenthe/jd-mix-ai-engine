#include "View2AutoTuneMatrix.h"
#include "../../core/ParamRegistry.h"

namespace jdmix::gui
{
    View2AutoTuneMatrix::View2AutoTuneMatrix(juce::AudioProcessorValueTreeState& state)
        : apvts(state)
    {
        // 1. Key & Scale Selectors
        keySelector.addItemList({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 1);
        scaleSelector.addItemList({ "Chromatic", "Major", "Natural Minor", "Harmonic Minor" }, 1);
        modeSelector.addItemList({ "Low Latency (Live)", "High Quality (HQ)" }, 1);

        addAndMakeVisible(keySelector);
        addAndMakeVisible(scaleSelector);
        addAndMakeVisible(modeSelector);

        keyAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, core::ParamIDs::pitchKey, keySelector);
        scaleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, core::ParamIDs::pitchScale, scaleSelector);
        modeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, core::ParamIDs::pitchMode, modeSelector);

        // 12-Note Pill Keyboard
        const char* noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        for (int i = 0; i < 12; ++i)
        {
            pillNotes[i].setButtonText(noteNames[i]);
            pillNotes[i].setColour(juce::TextButton::buttonColourId, ThemeColors::CyanNeon.withAlpha(0.2f));
            pillNotes[i].onClick = [this, i]()
            {
                noteActive[i] = !noteActive[i];
                pillNotes[i].setColour(juce::TextButton::buttonColourId,
                    noteActive[i] ? ThemeColors::CyanNeon.withAlpha(0.2f) : juce::Colours::transparentBlack);
                repaint();
            };
            addAndMakeVisible(pillNotes[i]);
        }

        // 2. Right Column Dials
        auto setupDial = [](juce::Slider& s, const juce::Colour& c)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            s.setColour(juce::Slider::rotarySliderFillColourId, c);
            s.setScrollWheelEnabled(true);
            s.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        };

        setupDial(retuneSpeedSlider, ThemeColors::CyanNeon);
        addAndMakeVisible(retuneSpeedSlider);

        setupDial(humanizeSlider, ThemeColors::PurpleNeon);
        addAndMakeVisible(humanizeSlider);

        setupDial(flexTuneSlider, ThemeColors::LimeNeon);
        addAndMakeVisible(flexTuneSlider);

        retuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::pitchRetuneSpeed, retuneSpeedSlider);
        humanizeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::pitchHumanize, humanizeSlider);
        flexTuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::pitchFlexTune, flexTuneSlider);

        modernClassicToggle.setColour(juce::ToggleButton::textColourId, ThemeColors::TextSecondary);
        addAndMakeVisible(modernClassicToggle);
    }

    View2AutoTuneMatrix::~View2AutoTuneMatrix() = default;

    void View2AutoTuneMatrix::updatePitchData(const juce::String& noteName, float centDeviation)
    {
        currentNote = noteName;
        currentCent = centDeviation;
        repaint();
    }

    void View2AutoTuneMatrix::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat().reduced(6.0f);
        auto leftCol = bounds.removeFromLeft(bounds.getWidth() * 0.48f);
        bounds.removeFromLeft(bounds.getWidth() * 0.04f);
        auto rightCol = bounds;

        // Draw Left Glass Panel
        g.setColour(ThemeColors::GraphitePanel);
        g.fillRoundedRectangle(leftCol, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(leftCol, 10.0f, 1.0f);

        // Subtle diagonal glass sheen
        juce::Path sheen;
        sheen.startNewSubPath(leftCol.getX(), leftCol.getY());
        sheen.lineTo(leftCol.getX() + leftCol.getWidth() * 0.4f, leftCol.getY());
        sheen.lineTo(leftCol.getX(), leftCol.getY() + leftCol.getHeight() * 0.5f);
        sheen.closeSubPath();
        g.setColour(juce::Colours::white.withAlpha(0.03f));
        g.fillPath(sheen);

        // AutoKey indicator + Big Central Note
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(ThemeColors::SafeEmerald);
        g.drawText("● AutoKey Listen (Local IPC Sync)", leftCol.getX() + 15.0f, leftCol.getY() + 45.0f, leftCol.getWidth() - 30.0f, 18.0f, juce::Justification::left);

        // Giant Note Display (F#)
        g.setFont(juce::Font(juce::FontOptions(54.0f, juce::Font::bold)));
        g.setColour(ThemeColors::TextPrimary);
        g.drawText(currentNote, leftCol.getX(), leftCol.getY() + 70.0f, leftCol.getWidth(), 65.0f, juce::Justification::centred);

        // Cent-Deviation Bar (-100, -50, 0, +50, +100)
        auto centBarArea = juce::Rectangle<float>(leftCol.getX() + 25.0f, leftCol.getY() + 145.0f, leftCol.getWidth() - 50.0f, 16.0f);
        g.setColour(ThemeColors::ObsidianBg);
        g.fillRoundedRectangle(centBarArea, 4.0f);

        // Center line
        g.setColour(ThemeColors::AmberGold);
        g.drawVerticalLine(static_cast<int>(centBarArea.getCentreX()), centBarArea.getY(), centBarArea.getBottom());

        // Lock indicator marker
        float markerX = centBarArea.getCentreX() + (currentCent / 100.0f) * (centBarArea.getWidth() * 0.48f);
        markerX = juce::jlimit(centBarArea.getX() + 4.0f, centBarArea.getRight() - 4.0f, markerX);

        g.setColour(ThemeColors::CyanNeon);
        g.fillEllipse(markerX - 4.0f, centBarArea.getY() + 4.0f, 8.0f, 8.0f);

        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("-100", centBarArea.getX(), centBarArea.getBottom() + 2.0f, 30.0f, 12.0f, juce::Justification::left);
        g.drawText("0", centBarArea.getCentreX() - 10.0f, centBarArea.getBottom() + 2.0f, 20.0f, 12.0f, juce::Justification::centred);
        g.drawText("+100", centBarArea.getRight() - 30.0f, centBarArea.getBottom() + 2.0f, 30.0f, 12.0f, juce::Justification::right);

        // Draw Right Glass Panel
        g.setColour(ThemeColors::GraphitePanel);
        g.fillRoundedRectangle(rightCol, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(rightCol, 10.0f, 1.0f);

        // Retune Speed readout
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.setColour(ThemeColors::CyanNeon);
        g.drawText("• " + juce::String(static_cast<int>(retuneSpeedSlider.getValue())) + " ms",
                   rightCol.getCentreX() - 40.0f, rightCol.getY() + 15.0f, 80.0f, 18.0f, juce::Justification::centred);

        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("RETUNE SPEED", rightCol.getCentreX() - 50.0f, rightCol.getY() + 140.0f, 100.0f, 16.0f, juce::Justification::centred);

        // Humanize & Flex Tune Labels
        auto botDialsArea = rightCol.removeFromBottom(rightCol.getHeight() * 0.45f);
        float halfW = botDialsArea.getWidth() * 0.5f;

        g.setColour(ThemeColors::PurpleNeon);
        g.drawText("• " + juce::String(static_cast<int>(humanizeSlider.getValue())) + "%",
                   botDialsArea.getX() + 10.0f, botDialsArea.getY() + 5.0f, halfW - 20.0f, 16.0f, juce::Justification::centred);
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("HUMANIZE", botDialsArea.getX() + 10.0f, botDialsArea.getBottom() - 25.0f, halfW - 20.0f, 16.0f, juce::Justification::centred);

        g.setColour(ThemeColors::LimeNeon);
        g.drawText("• " + juce::String(static_cast<int>(flexTuneSlider.getValue())) + "%",
                   botDialsArea.getX() + halfW + 10.0f, botDialsArea.getY() + 5.0f, halfW - 20.0f, 16.0f, juce::Justification::centred);
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("FLEX TUNE", botDialsArea.getX() + halfW + 10.0f, botDialsArea.getBottom() - 25.0f, halfW - 20.0f, 16.0f, juce::Justification::centred);
    }

    void View2AutoTuneMatrix::resized()
    {
        auto bounds = getLocalBounds().reduced(10);
        auto leftCol = bounds.removeFromLeft(bounds.getWidth() * 0.48f);
        bounds.removeFromLeft(bounds.getWidth() * 0.04f);
        auto rightCol = bounds;

        // Top dropdowns on left
        int dropW = (leftCol.getWidth() - 20) / 2;
        keySelector.setBounds(leftCol.getX() + 10, leftCol.getY() + 10, dropW, 26);
        scaleSelector.setBounds(leftCol.getX() + 15 + dropW, leftCol.getY() + 10, dropW, 26);

        // 12-Note Pill Keyboard at bottom of left column
        int pillW = (leftCol.getWidth() - 20) / 6;
        int pillH = 24;
        int pillY = leftCol.getBottom() - 70;

        for (int i = 0; i < 6; ++i)
            pillNotes[i].setBounds(leftCol.getX() + 10 + i * pillW, pillY, pillW - 2, pillH);
        for (int i = 6; i < 12; ++i)
            pillNotes[i].setBounds(leftCol.getX() + 10 + (i - 6) * pillW, pillY + pillH + 4, pillW - 2, pillH);

        modeSelector.setBounds(leftCol.getX() + 10, leftCol.getBottom() - 28, leftCol.getWidth() - 20, 22);

        // Giant Retune Dial
        retuneSpeedSlider.setBounds(rightCol.getCentreX() - 55, rightCol.getY() + 30, 110, 110);

        // Bottom two dials
        int bSize = 75;
        int midY = rightCol.getBottom() - 110;
        humanizeSlider.setBounds(rightCol.getX() + 20, midY, bSize, bSize);
        flexTuneSlider.setBounds(rightCol.getRight() - bSize - 20, midY, bSize, bSize);

        modernClassicToggle.setBounds(rightCol.getCentreX() - 60, rightCol.getBottom() - 25, 120, 20);
    }
}
