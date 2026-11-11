#include "View4SpatialSends.h"
#include "../../core/ParamRegistry.h"
#include <cmath>

namespace jdmix::gui
{
    View4SpatialSends::View4SpatialSends(juce::AudioProcessorValueTreeState& state)
        : apvts(state)
    {
        auto setupDial = [this](juce::Slider& s, const juce::Colour& colour)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            s.setColour(juce::Slider::rotarySliderFillColourId, colour);
            s.setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
            addAndMakeVisible(s);
        };

        setupDial(preDelaySlider,  ThemeColors::LimeNeon);
        setupDial(slapTimeSlider,  ThemeColors::LimeNeon);
        setupDial(plateSendSlider, ThemeColors::CyanNeon);
        setupDial(slapSendSlider,  ThemeColors::CyanNeon);

        setupDial(mixInputSlider,   ThemeColors::TextSecondary);
        setupDial(mixPanner1Slider, ThemeColors::CyanNeon);
        setupDial(mixPanner2Slider, ThemeColors::LimeNeon);
        setupDial(mixOutputSlider,  ThemeColors::TextSecondary);
        setupDial(mixBlendSlider,   ThemeColors::AmberGold);

        preDelayAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::sendPlatePredelay, preDelaySlider);
        slapTimeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::sendSlapTime, slapTimeSlider);
        plateSendAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::sendPlateSend, plateSendSlider);
        slapSendAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, core::ParamIDs::sendSlapSend, slapSendSlider);
    }

    View4SpatialSends::~View4SpatialSends() = default;

    void View4SpatialSends::mouseDown(const juce::MouseEvent& e)
    {
        lastMousePos = e.position;
        if (cachedGlobeArea.contains(e.position))
        {
            clickRipplePos = e.position;
            clickRippleRadius = 8.0f;
            clickRippleAlpha = 0.95f;
            repaint();
        }
    }

    void View4SpatialSends::mouseDrag(const juce::MouseEvent& e)
    {
        if (cachedGlobeArea.contains(e.position) || cachedGlobeArea.contains(lastMousePos))
        {
            auto delta = e.position - lastMousePos;
            sphereYaw += delta.x * 0.012f;
            spherePitch = juce::jlimit(-1.35f, 1.35f, spherePitch + delta.y * 0.012f);
            lastMousePos = e.position;
            repaint();
        }
    }

    void View4SpatialSends::mouseMove(const juce::MouseEvent& e)
    {
        float normX = (e.position.x - getWidth() * 0.5f) / (getWidth() * 0.5f);
        float normY = (e.position.y - getHeight() * 0.5f) / (getHeight() * 0.5f);
        cardTiltX = juce::jlimit(-1.0f, 1.0f, normX);
        cardTiltY = juce::jlimit(-1.0f, 1.0f, normY);

        if (cachedGlobeArea.contains(e.position))
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        else
            setMouseCursor(juce::MouseCursor::NormalCursor);

        repaint();
    }

    void View4SpatialSends::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
    {
        if (cachedGlobeArea.contains(e.position))
        {
            sphereZoom = juce::jlimit(0.65f, 1.75f, sphereZoom + wheel.deltaY * 0.15f);
            repaint();
        }
        else
        {
            Component::mouseWheelMove(e, wheel);
        }
    }

    void View4SpatialSends::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat().reduced(6.0f);
        auto leftStrips = bounds.removeFromLeft(bounds.getWidth() * 0.42f);
        bounds.removeFromLeft(bounds.getWidth() * 0.03f);
        auto rightArea = bounds;

        // Draw Left Modular Channel Strip with 3D Parallax Tilt Sheen
        g.setColour(ThemeColors::GraphitePanel);
        g.fillRoundedRectangle(leftStrips, 10.0f);

        // Specular dynamic light catch moving with cardTiltX/Y
        float sheenCenterX = leftStrips.getX() + leftStrips.getWidth() * (0.5f + cardTiltX * 0.35f);
        float sheenCenterY = leftStrips.getY() + leftStrips.getHeight() * (0.5f + cardTiltY * 0.35f);
        juce::ColourGradient cardSheen(juce::Colours::white.withAlpha(0.06f), sheenCenterX, sheenCenterY,
                                      juce::Colours::transparentWhite, leftStrips.getRight(), leftStrips.getBottom(), true);
        g.setGradientFill(cardSheen);
        g.fillRoundedRectangle(leftStrips, 10.0f);

        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.drawRoundedRectangle(leftStrips, 10.0f, 1.0f);

        // Titles on Left Strip
        g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        g.setColour(ThemeColors::TextPrimary);
        g.drawText("PANNER 1 / SEND 1 (PLATE)", leftStrips.getX() + 12.0f, leftStrips.getY() + 10.0f, 220.0f, 16.0f, juce::Justification::left);
        g.drawText("PANNER 2 / SEND 2 (SLAP)",  leftStrips.getX() + 12.0f, leftStrips.getY() + 115.0f, 220.0f, 16.0f, juce::Justification::left);

        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.setColour(ThemeColors::TextSecondary);
        g.drawText("PREDLY\n20-40ms", leftStrips.getX() + 10.0f, leftStrips.getY() + 75.0f, 65.0f, 24.0f, juce::Justification::centred);
        g.drawText("PLATE\nLEVEL", leftStrips.getX() + 85.0f, leftStrips.getY() + 75.0f, 65.0f, 24.0f, juce::Justification::centred);

        g.drawText("MOD DLY\n80-120ms", leftStrips.getX() + 10.0f, leftStrips.getY() + 180.0f, 65.0f, 24.0f, juce::Justification::centred);
        g.drawText("SLAP\nLEVEL", leftStrips.getX() + 85.0f, leftStrips.getY() + 180.0f, 65.0f, 24.0f, juce::Justification::centred);

        // Right Upper: 3D Wireframe Globe Area
        auto globeArea = rightArea.removeFromTop(rightArea.getHeight() * 0.65f);
        cachedGlobeArea = globeArea;
        draw3dWireframeSphere(g, globeArea);

        // Right Lower: Hardware Mixer Bay
        g.setColour(juce::Colour(0xff0d101a));
        g.fillRoundedRectangle(rightArea, 8.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(rightArea, 8.0f, 1.0f);

        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.setColour(ThemeColors::TextPrimary);
        g.drawText("HARDWARE MIXER BAY", rightArea.getX() + 10.0f, rightArea.getY() + 6.0f, 150.0f, 14.0f, juce::Justification::left);

        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.setColour(ThemeColors::SafeEmerald);
        g.drawText("EXT. S.C. DUCKING: ON | 3D GYRO: ACTIVE", rightArea.getRight() - 250.0f, rightArea.getY() + 6.0f, 240.0f, 14.0f, juce::Justification::right);
    }

    void View4SpatialSends::draw3dWireframeSphere(juce::Graphics& g, juce::Rectangle<float> area)
    {
        // Dark Obsidian Horizon
        g.setColour(ThemeColors::ObsidianBg);
        g.fillRoundedRectangle(area, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(area, 10.0f, 1.0f);

        auto centre = area.getCentre();
        float baseRadius = juce::jmin(area.getWidth(), area.getHeight()) * 0.35f;
        float radius = baseRadius * sphereZoom;

        // Glowing Pedestal Base with Metallic Specular Ring
        g.setColour(ThemeColors::CyanNeon.withAlpha(0.18f * sphereZoom));
        g.fillEllipse(centre.x - radius * 0.95f, centre.y + radius * 0.72f, radius * 1.9f, 26.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawEllipse(centre.x - radius * 0.95f, centre.y + radius * 0.72f, radius * 1.9f, 26.0f, 1.2f);

        // 3D Point Projection Helper
        auto projectPoint = [centre, radius, this](float theta, float phi) -> juce::Point<float>
        {
            // Unit sphere coordinates
            float x0 = std::cos(phi) * std::sin(theta);
            float y0 = std::sin(phi);
            float z0 = std::cos(phi) * std::cos(theta);

            // Rotate around Y-axis (Yaw)
            float x1 = x0 * std::cos(sphereYaw) + z0 * std::sin(sphereYaw);
            float z1 = -x0 * std::sin(sphereYaw) + z0 * std::cos(sphereYaw);

            // Rotate around X-axis (Pitch)
            float y2 = y0 * std::cos(spherePitch) - z1 * std::sin(spherePitch);
            float z2 = y0 * std::sin(spherePitch) + z1 * std::cos(spherePitch);

            // Perspective factor
            float k = 1.0f / (1.0f + z2 * 0.28f);
            return { centre.x + x1 * radius * k, centre.y + y2 * radius * k };
        };

        // Outer Silhouette Ring
        g.setColour(juce::Colours::white.withAlpha(0.14f));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        // 3D Latitude Rings
        constexpr int segs = 48;
        float latAngles[5] = { -1.05f, -0.52f, 0.0f, 0.52f, 1.05f };
        for (float phi : latAngles)
        {
            juce::Path latPath;
            for (int s = 0; s <= segs; ++s)
            {
                float theta = (s / static_cast<float>(segs)) * juce::MathConstants<float>::twoPi;
                auto pt = projectPoint(theta, phi);
                if (s == 0) latPath.startNewSubPath(pt);
                else        latPath.lineTo(pt);
            }
            latPath.closeSubPath();
            float alpha = (std::abs(phi) < 0.1f) ? 0.28f : 0.12f;
            g.setColour(ThemeColors::CyanNeon.withAlpha(alpha));
            g.strokePath(latPath, juce::PathStrokeType(1.0f));
        }

        // 3D Longitude Meridians
        float lonAngles[6] = { 0.0f, 0.52f, 1.05f, 1.57f, 2.09f, 2.62f };
        for (float theta : lonAngles)
        {
            juce::Path lonPath;
            for (int s = 0; s <= segs; ++s)
            {
                float phi = -juce::MathConstants<float>::halfPi + (s / static_cast<float>(segs)) * juce::MathConstants<float>::pi;
                auto pt = projectPoint(theta, phi);
                if (s == 0) lonPath.startNewSubPath(pt);
                else        lonPath.lineTo(pt);
            }
            g.setColour(juce::Colours::white.withAlpha(0.08f));
            g.strokePath(lonPath, juce::PathStrokeType(0.8f));
        }

        // 3D Interactive Nodes (START, A, B)
        struct Node3D { const char* label; float theta; float phi; juce::Colour col; };
        Node3D nodes[3] = {
            { "START", 0.0f, 0.0f, ThemeColors::CyanNeon },
            { "A", 1.1f, 0.45f, ThemeColors::AmberGold },
            { "B", -1.3f, -0.4f, ThemeColors::SafeEmerald }
        };

        for (const auto& n : nodes)
        {
            auto pt = projectPoint(n.theta, n.phi);

            // Depth calculation for node size & glow
            float x0 = std::cos(n.phi) * std::sin(n.theta);
            float y0 = std::sin(n.phi);
            float z0 = std::cos(n.phi) * std::cos(n.theta);
            float z1 = -x0 * std::sin(sphereYaw) + z0 * std::cos(sphereYaw);
            float z2 = y0 * std::sin(spherePitch) + z1 * std::cos(spherePitch);

            float nodeScale = juce::jmap(z2, -1.0f, 1.0f, 0.7f, 1.35f);
            float nodeAlpha = juce::jmap(z2, -1.0f, 1.0f, 0.35f, 1.0f);

            float sz = 14.0f * nodeScale;
            g.setColour(n.col.withAlpha(nodeAlpha * 0.35f));
            g.fillEllipse(pt.x - sz * 0.9f, pt.y - sz * 0.9f, sz * 1.8f, sz * 1.8f);

            g.setColour(n.col.withAlpha(nodeAlpha));
            g.fillEllipse(pt.x - sz * 0.5f, pt.y - sz * 0.5f, sz, sz);

            g.setColour(juce::Colours::black);
            g.setFont(juce::Font(juce::FontOptions(8.5f * nodeScale, juce::Font::bold)));
            g.drawText(n.label, pt.x - sz * 0.5f, pt.y - sz * 0.5f, sz, sz, juce::Justification::centred);
        }

        // Click Sonic Shockwave Ripple
        if (clickRippleAlpha > 0.02f)
        {
            g.setColour(ThemeColors::CyanNeon.withAlpha(clickRippleAlpha));
            g.drawEllipse(clickRipplePos.x - clickRippleRadius,
                          clickRipplePos.y - clickRippleRadius * 0.65f,
                          clickRippleRadius * 2.0f,
                          clickRippleRadius * 1.3f, 2.0f);
            clickRippleRadius += 2.8f;
            clickRippleAlpha -= 0.045f;
            repaint();
        }

        // Interaction Overlay Hint
        g.setFont(juce::Font(juce::FontOptions(9.5f)));
        g.setColour(ThemeColors::TextSecondary.withAlpha(0.7f));
        g.drawText("⚡ DRAG TO ROTATE 3D SPHERE | SCROLL TO ZOOM", area.getX() + 10.0f, area.getBottom() - 18.0f, area.getWidth() - 20.0f, 14.0f, juce::Justification::centred);
    }

    void View4SpatialSends::resized()
    {
        auto bounds = getLocalBounds().reduced(10);
        auto leftStrips = bounds.removeFromLeft(bounds.getWidth() * 0.42f);
        bounds.removeFromLeft(bounds.getWidth() * 0.03f);
        auto rightArea = bounds;

        // Panner 1 knobs
        preDelaySlider.setBounds(leftStrips.getX() + 15, leftStrips.getY() + 30, 50, 50);
        plateSendSlider.setBounds(leftStrips.getX() + 90, leftStrips.getY() + 30, 50, 50);

        // Panner 2 knobs
        slapTimeSlider.setBounds(leftStrips.getX() + 15, leftStrips.getY() + 135, 50, 50);
        slapSendSlider.setBounds(leftStrips.getX() + 90, leftStrips.getY() + 135, 50, 50);

        // Hardware Mixer Bay Dials
        auto mixerBay = rightArea.removeFromBottom(rightArea.getHeight() * 0.35f);
        int mDialW = mixerBay.getWidth() / 5;
        int mY = mixerBay.getY() + 22;

        mixInputSlider.setBounds(mixerBay.getX(), mY, mDialW, 45);
        mixPanner1Slider.setBounds(mixerBay.getX() + mDialW, mY, mDialW, 45);
        mixPanner2Slider.setBounds(mixerBay.getX() + mDialW * 2, mY, mDialW, 45);
        mixOutputSlider.setBounds(mixerBay.getX() + mDialW * 3, mY, mDialW, 45);
        mixBlendSlider.setBounds(mixerBay.getX() + mDialW * 4, mY, mDialW, 45);
    }
}
