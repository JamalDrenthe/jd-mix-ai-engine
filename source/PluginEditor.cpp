#include "PluginEditor.h"

JDMixAIEngineAudioProcessorEditor::JDMixAIEngineAudioProcessorEditor(JDMixAIEngineAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      view1(p.getAPVTS()),
      view2(p.getAPVTS()),
      view3(p.getAPVTS(), p.getAiClient()),
      view4(p.getAPVTS()),
      view5(p.getAPVTS(), p.getLocalIpc())
{
    setLookAndFeel(&lookAndFeel);

    // Track Role Selector
    trackRoleSelector.addItemList({ "Role: Lead Vocal", "Role: Backing Vocal", "Role: Ad-Lib", "Role: Harmony",
                                    "Role: Drums (Bus)", "Role: Kick", "Role: Snare", "Role: Hi-Hats",
                                    "Role: 808 / Bass", "Role: Synth", "Role: Guitar", "Role: Piano",
                                    "Role: Vocal Bus", "Role: Instrument Bus", "Role: Master" }, 1);
    addAndMakeVisible(trackRoleSelector);
    trackRoleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.getAPVTS(), jdmix::core::ParamIDs::trackRole, trackRoleSelector);

    // 5-Tab Navigation
    const char* tabNames[5] = { "EQ & Decay", "Auto-Tune", "AI Orb", "3D Space", "Stem Radar" };
    for (int i = 0; i < 5; ++i)
    {
        viewTabs[i].setButtonText(tabNames[i]);
        viewTabs[i].setColour(juce::TextButton::buttonColourId, (i == 0) ? jdmix::gui::ThemeColors::CyanNeon.withAlpha(0.35f) : jdmix::gui::ThemeColors::GraphitePanel);
        viewTabs[i].onClick = [this, i]() { updateActiveView(i); };
        addAndMakeVisible(viewTabs[i]);
    }

    // A / B / Delta Auditioning
    aSlotButton.setColour(juce::TextButton::buttonColourId, jdmix::gui::ThemeColors::CyanNeon.withAlpha(0.4f));
    aSlotButton.onClick = [this]()
    {
        processor.getAiClient().toggleSlotB(false);
        aSlotButton.setColour(juce::TextButton::buttonColourId, jdmix::gui::ThemeColors::CyanNeon.withAlpha(0.4f));
        bSlotButton.setColour(juce::TextButton::buttonColourId, jdmix::gui::ThemeColors::GraphitePanel);
    };
    addAndMakeVisible(aSlotButton);

    bSlotButton.onClick = [this]()
    {
        processor.getAiClient().toggleSlotB(true);
        bSlotButton.setColour(juce::TextButton::buttonColourId, jdmix::gui::ThemeColors::PurpleNeon.withAlpha(0.4f));
        aSlotButton.setColour(juce::TextButton::buttonColourId, jdmix::gui::ThemeColors::GraphitePanel);
    };
    addAndMakeVisible(bSlotButton);

    deltaButton.onClick = [this]()
    {
        bool nextDelta = !processor.getAiClient().isDeltaActive();
        processor.getAiClient().toggleDeltaMode(nextDelta);
        deltaButton.setColour(juce::TextButton::buttonColourId,
            nextDelta ? jdmix::gui::ThemeColors::AmberGold.withAlpha(0.5f) : jdmix::gui::ThemeColors::GraphitePanel);
    };
    addAndMakeVisible(deltaButton);

    // Update Checker Pill
    updatePillButton.setColour(juce::TextButton::buttonColourId, jdmix::gui::ThemeColors::PurpleNeon.withAlpha(0.6f));
    updatePillButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    updatePillButton.onClick = [this]()
    {
        juce::URL(updateChecker.getDownloadUrl()).launchInDefaultBrowser();
    };
    addChildComponent(updatePillButton);

    updateChecker.checkForUpdatesAsync([this](bool available, juce::String latestVer, juce::String dlUrl)
    {
        juce::ignoreUnused(dlUrl);
        if (available)
        {
            updatePillButton.setButtonText("🚀 Update " + latestVer);
            updatePillButton.setVisible(true);
            resized();
        }
    });

    // AI Execute hook on View 3
    view3.onExecuteAiPlan = [this]()
    {
        auto snapshot = processor.getMirExtractor().getLatestSnapshot();
        processor.getAiClient().triggerAutoOptimize(snapshot, "Modern Trapsoul Master Chain", 100.0f);
    };

    // Add Workspaces
    addChildComponent(view1);
    addChildComponent(view2);
    addChildComponent(view3);
    addChildComponent(view4);
    addChildComponent(view5);

    updateActiveView(0);

    setSize(920, 580);
    startTimerHz(60); // 60 FPS UI refresh
}

JDMixAIEngineAudioProcessorEditor::~JDMixAIEngineAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void JDMixAIEngineAudioProcessorEditor::updateActiveView(int newIndex)
{
    activeViewIndex = newIndex;
    view1.setVisible(activeViewIndex == 0);
    view2.setVisible(activeViewIndex == 1);
    view3.setVisible(activeViewIndex == 2);
    view4.setVisible(activeViewIndex == 3);
    view5.setVisible(activeViewIndex == 4);

    for (int i = 0; i < 5; ++i)
    {
        viewTabs[i].setColour(juce::TextButton::buttonColourId,
            (i == activeViewIndex) ? jdmix::gui::ThemeColors::CyanNeon.withAlpha(0.35f) : jdmix::gui::ThemeColors::GraphitePanel);
    }
}

void JDMixAIEngineAudioProcessorEditor::timerCallback()
{
    // Update Spectrum & meters on active view
    processor.getSpectrum().updateSpectrum();

    if (activeViewIndex == 0)
    {
        std::array<float, jdmix::analysis::SpectrumAnalyzer::numBands64> spec64;
        processor.getSpectrum().get64Bands(spec64);

        std::array<float, jdmix::ipc::ipcSpectrumBands> maskSpec;
        bool hasMask = processor.getLocalIpc().getMaskingSpectrum(3 /* Kick */, maskSpec);

        view1.updateSpectrum(spec64, hasMask ? &maskSpec : nullptr);
    }
    else if (activeViewIndex == 1)
    {
        const char* noteClasses[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int noteIdx = processor.getPitchStudio().getDetectedNoteIndex();
        float cent = processor.getPitchStudio().getCentDeviation();
        view2.updatePitchData(noteClasses[noteIdx], cent);
    }
    else if (activeViewIndex == 2)
    {
        float formant = *processor.getAPVTS().getRawParameterValue(jdmix::core::ParamIDs::pitchFormantShift);
        view3.setAudioTelemetry(processor.getRmsInput(), formant / 12.0f);
    }
    else if (activeViewIndex == 4)
    {
        view5.updateRadarData();
    }

    repaint(0, getHeight() - 30, getWidth(), 30); // Repaint footer meters
}

void JDMixAIEngineAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Top Header Background
    auto headerArea = bounds.removeFromTop(44.0f);
    g.setColour(jdmix::gui::ThemeColors::ObsidianBg);
    g.fillRect(headerArea);

    // Draw Canonical JD Logo Mark in Header
    jdmix::gui::JDMixLookAndFeel::drawJdLogoMark(g, juce::Rectangle<float>(14.0f, 10.0f, 26.0f, 24.0f), jdmix::gui::ThemeColors::CyanNeon);

    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.setColour(jdmix::gui::ThemeColors::TextPrimary);
    g.drawText("JD MIX AI ENGINE", 46.0f, headerArea.getY() + 11.0f, 150.0f, 22.0f, juce::Justification::left);

    // Online badge & instances count
    int activeInstances = processor.getLocalIpc().getActiveInstanceCount();
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.setColour(jdmix::gui::ThemeColors::SafeEmerald);
    g.drawText("● ONLINE | " + juce::String(activeInstances) + " TRACKS LINKED", 205.0f, headerArea.getY() + 13.0f, 160.0f, 18.0f, juce::Justification::left);

    // Bottom Footer Background
    auto footerArea = getLocalBounds().toFloat().removeFromBottom(30.0f);
    g.setColour(jdmix::gui::ThemeColors::ObsidianBg);
    g.fillRect(footerArea);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawHorizontalLine(static_cast<int>(footerArea.getY()), 0.0f, static_cast<float>(getWidth()));

    // Meters Readout in Footer
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.setColour(jdmix::gui::ThemeColors::TextSecondary);

    float inDb  = juce::Decibels::gainToDecibels(processor.getRmsInput(), -80.0f);
    float outDb = juce::Decibels::gainToDecibels(processor.getRmsOutput(), -80.0f);
    float grDb  = processor.getGainReduction();
    float lufsS = processor.getLufsMeter().getShortTermLufs();

    juce::String meterStr = "IN: " + juce::String(inDb, 1) + " dBFS | OUT: " + juce::String(outDb, 1) + " dBFS"
                          + " | GR: " + juce::String(grDb, 1) + " dB | LUFS-S: " + juce::String(lufsS, 1)
                          + " | FL STUDIO PDC: " + juce::String(processor.getLatencySamples()) + " smp (LOCKED)";

    g.drawText(meterStr, footerArea.getX() + 15.0f, footerArea.getY() + 6.0f, footerArea.getWidth() - 140.0f, 18.0f, juce::Justification::left);

    // Footer JD Logo Brand Stamp
    jdmix::gui::JDMixLookAndFeel::drawJdLogoMark(g, juce::Rectangle<float>(footerArea.getRight() - 118.0f, footerArea.getY() + 7.0f, 16.0f, 16.0f), juce::Colours::white.withAlpha(0.45f));
    g.setColour(jdmix::gui::ThemeColors::TextSecondary);
    g.drawText("JD Audio Labs", footerArea.getRight() - 98.0f, footerArea.getY() + 6.0f, 85.0f, 18.0f, juce::Justification::left);
}

void JDMixAIEngineAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(44).reduced(6);
    auto footer = bounds.removeFromBottom(30);

    if (updatePillButton.isVisible())
    {
        updatePillButton.setBounds(header.getX() + 325, header.getY() + 3, 140, 26);
        trackRoleSelector.setBounds(header.getX() + 472, header.getY() + 3, 130, 26);
    }
    else
    {
        trackRoleSelector.setBounds(header.getX() + 340, header.getY() + 3, 140, 26);
    }

    // Tabs
    int tabW = 80;
    int tabStart = updatePillButton.isVisible() ? (header.getX() + 610) : (header.getX() + 490);
    for (int i = 0; i < 5; ++i)
    {
        viewTabs[i].setBounds(tabStart + i * (tabW + 3), header.getY() + 3, tabW, 26);
    }

    // A / B / Delta buttons
    aSlotButton.setBounds(header.getRight() - 85, header.getY() + 3, 24, 26);
    bSlotButton.setBounds(header.getRight() - 58, header.getY() + 3, 24, 26);
    deltaButton.setBounds(header.getRight() - 30, header.getY() + 3, 24, 26);

    // Workspace Canvas
    view1.setBounds(bounds);
    view2.setBounds(bounds);
    view3.setBounds(bounds);
    view4.setBounds(bounds);
    view5.setBounds(bounds);
}
