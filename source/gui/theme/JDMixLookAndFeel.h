#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace jdmix::gui
{
    struct ThemeColors
    {
        static inline const juce::Colour ObsidianBg    { 0xff090a0f };
        static inline const juce::Colour GraphitePanel { 0xff12151f };
        static inline const juce::Colour SurfaceElevated{ 0xff1a1d2b };
        static inline const juce::Colour CyanNeon      { 0xff00e5ff }; // View 1 & View 2 Retune Arc
        static inline const juce::Colour PurpleNeon    { 0xffa855f7 }; // View 3 Radar & Humanize Arc
        static inline const juce::Colour LimeNeon      { 0xff84cc16 }; // View 2 Flex-Tune Arc
        static inline const juce::Colour AmberGold     { 0xfff59e0b }; // View 1 Post-EQ Curve
        static inline const juce::Colour WarningRed    { 0xfff43f5e }; // Masking collision
        static inline const juce::Colour SafeEmerald   { 0xff10b981 };
        static inline const juce::Colour TextPrimary   { 0xffffffff };
        static inline const juce::Colour TextSecondary { 0xff94a3b8 };
    };

    class JDMixLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        // Canonical JD Logo Mark (from icon.svg / 256x162 canonical source)
        static void drawJdLogoMark(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour = ThemeColors::TextPrimary)
        {
            juce::Path p = juce::Drawable::parseSVGPath(
                "M5 7h10v4H9v9c0 3-2 5-5 5-2 0-3-.7-4-2l3-3c.4.7.8 1 1.4 1 .9 0 1.6-.8 1.6-2V7Zm12 0h6c6 0 9 3.4 9 9s-3 9-9 9h-6V7Zm4 4v10h2c3.2 0 5-1.5 5-5s-1.8-5-5-5h-2Z");

            auto pBounds = p.getBounds();
            if (pBounds.isEmpty())
                return;

            float scale = juce::jmin(bounds.getWidth() / pBounds.getWidth(), bounds.getHeight() / pBounds.getHeight());
            auto transform = juce::AffineTransform::translation(-pBounds.getX(), -pBounds.getY())
                                .scaled(scale)
                                .translated(bounds.getX() + (bounds.getWidth() - pBounds.getWidth() * scale) * 0.5f,
                                            bounds.getY() + (bounds.getHeight() - pBounds.getHeight() * scale) * 0.5f);

            g.setColour(colour);
            g.fillPath(p, transform);
        }

        JDMixLookAndFeel()
        {
            setColour(juce::ResizableWindow::backgroundColourId, ThemeColors::ObsidianBg);
            setColour(juce::Label::textColourId, ThemeColors::TextPrimary);
            setColour(juce::ComboBox::backgroundColourId, ThemeColors::GraphitePanel);
            setColour(juce::ComboBox::textColourId, ThemeColors::TextPrimary);
            setColour(juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha(0.12f));
            setColour(juce::TextButton::buttonColourId, ThemeColors::GraphitePanel);
            setColour(juce::TextButton::textColourOffId, ThemeColors::TextPrimary);
        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, const float rotaryStartAngle,
                              const float rotaryEndAngle, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6.0f);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto centre = bounds.getCentre();
            auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            bool isOver = slider.isMouseOverOrDragging();
            bool isDown = slider.isMouseButtonDown();

            // Tactile 3D physical scaling (0.96 on click press, 1.025 on hover)
            float tactileScale = isDown ? 0.96f : (isOver ? 1.025f : 1.0f);
            float bodyRadius = radius * 0.76f * tactileScale;

            // 1. Dark matte-obsidian knob body with ambient drop-shadow
            if (isOver)
            {
                g.setColour(juce::Colours::black.withAlpha(0.4f));
                g.fillEllipse(centre.x - bodyRadius, centre.y - bodyRadius + 2.5f, bodyRadius * 2.0f, bodyRadius * 2.0f);
            }

            g.setColour(juce::Colour(isOver ? 0xff1f2430 : 0xff181c26));
            g.fillEllipse(centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);

            // 2. Luxury 3D Specular Sheen (Atelier light catch)
            juce::ColourGradient specularGrad(
                juce::Colours::white.withAlpha(isOver ? 0.22f : 0.09f),
                centre.x - bodyRadius * 0.4f, centre.y - bodyRadius * 0.4f,
                juce::Colours::transparentWhite,
                centre.x + bodyRadius * 0.5f, centre.y + bodyRadius * 0.5f,
                true);
            g.setGradientFill(specularGrad);
            g.fillEllipse(centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);

            // Hairline specular ring
            g.setColour(juce::Colours::white.withAlpha(isOver ? 0.16f : 0.08f));
            g.drawEllipse(centre.x - bodyRadius, centre.y - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.2f);

            // 3. Outer Glowing Neon Value Arc
            juce::Colour arcColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
            if (arcColour.isTransparent())
                arcColour = ThemeColors::CyanNeon;

            float arcRadius = radius * (isOver ? 0.89f : 0.87f);
            juce::Path trackArc;
            trackArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                   0.0f, rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(juce::Colours::white.withAlpha(0.06f));
            g.strokePath(trackArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            juce::Path valueArc;
            valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                   0.0f, rotaryStartAngle, toAngle, true);

            // Dynamic glow dilation on hover
            float glowWidth = isOver ? 9.0f : 5.5f;
            float glowAlpha = isOver ? 0.42f : 0.28f;
            g.setColour(arcColour.withAlpha(glowAlpha));
            g.strokePath(valueArc, juce::PathStrokeType(glowWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            g.setColour(arcColour);
            g.strokePath(valueArc, juce::PathStrokeType(isOver ? 2.8f : 2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // 4. Pointer needle with luminous tip
            juce::Path pointer;
            pointer.addRoundedRectangle(-1.5f, -bodyRadius * 0.92f, 3.0f, bodyRadius * 0.35f, 1.5f);
            g.setColour(isOver ? arcColour : juce::Colours::white);
            g.fillPath(pointer, juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
        }

        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
            auto cornerSize = 7.0f;

            // Tactile 3D press depression
            if (shouldDrawButtonAsDown)
                bounds = bounds.translated(0.0f, 1.0f);
            else if (shouldDrawButtonAsHighlighted)
                bounds = bounds.translated(0.0f, -1.0f);

            juce::Colour base = backgroundColour;
            if (shouldDrawButtonAsDown)
                base = base.darker(0.1f);
            else if (shouldDrawButtonAsHighlighted)
                base = base.brighter(0.25f);

            // Button ambient drop glow on hover
            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour(base.withAlpha(0.25f));
                g.fillRoundedRectangle(bounds.translated(0.0f, 3.0f), cornerSize);
            }

            // Glassmorphism button chassis
            g.setColour(base);
            g.fillRoundedRectangle(bounds, cornerSize);

            // Specular top highlight (Apple Glass Hairline)
            g.setColour(juce::Colours::white.withAlpha(shouldDrawButtonAsHighlighted ? 0.22f : 0.10f));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

            // Top edge sheen
            g.setColour(juce::Colours::white.withAlpha(shouldDrawButtonAsHighlighted ? 0.30f : 0.15f));
            g.drawLine(bounds.getX() + cornerSize, bounds.getY() + 1.0f,
                       bounds.getRight() - cornerSize, bounds.getY() + 1.0f, 1.0f);
        }
    };
}
