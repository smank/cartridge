#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace cart::ui
{

// Global LookAndFeel for Cartridge. Routes JUCE's stock controls (sliders,
// combos, toggles, popups) through Cartridge's palette so every component
// reads as one product.
class CartridgeLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    CartridgeLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, Palette::background);

        setColour (juce::Slider::rotarySliderFillColourId,    Palette::primary);
        setColour (juce::Slider::rotarySliderOutlineColourId, Palette::outline);
        setColour (juce::Slider::thumbColourId,               Palette::secondary);
        setColour (juce::Slider::trackColourId,               Palette::primary);
        setColour (juce::Slider::backgroundColourId,          Palette::outlineDim);
        setColour (juce::Slider::textBoxTextColourId,         Palette::textPrimary);
        setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);

        setColour (juce::Label::textColourId,                 Palette::textPrimary);
        setColour (juce::Label::backgroundColourId,           juce::Colours::transparentBlack);

        setColour (juce::ComboBox::backgroundColourId,        Palette::surfaceHi);
        setColour (juce::ComboBox::outlineColourId,           Palette::outline);
        setColour (juce::ComboBox::textColourId,              Palette::textPrimary);
        setColour (juce::ComboBox::arrowColourId,             Palette::primary);
        setColour (juce::ComboBox::focusedOutlineColourId,    Palette::hot);

        setColour (juce::PopupMenu::backgroundColourId,           Palette::surface);
        setColour (juce::PopupMenu::textColourId,                 Palette::textPrimary);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::primaryDim);
        setColour (juce::PopupMenu::highlightedTextColourId,      Palette::textPrimary);

        setColour (juce::ToggleButton::textColourId,         Palette::textPrimary);
        setColour (juce::ToggleButton::tickColourId,         Palette::primary);
        setColour (juce::ToggleButton::tickDisabledColourId, Palette::textDim);

        setColour (juce::TextButton::buttonColourId,         Palette::surfaceHi);
        setColour (juce::TextButton::buttonOnColourId,       Palette::primaryDim);
        setColour (juce::TextButton::textColourOffId,        Palette::textPrimary);
        setColour (juce::TextButton::textColourOnId,         Palette::textPrimary);

        setColour (juce::TooltipWindow::backgroundColourId,  Palette::surface);
        setColour (juce::TooltipWindow::textColourId,        Palette::textPrimary);
        setColour (juce::TooltipWindow::outlineColourId,     Palette::outline);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStart, float rotaryEnd,
                           juce::Slider& slider) override
    {
        using namespace juce;
        const auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        const float radius = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        if (radius < 6.0f) return;

        const Point<float> centre = bounds.getCentre();
        const float strokeWidth = Metrics::knobStrokeWidth;
        const float angle = rotaryStart + sliderPos * (rotaryEnd - rotaryStart);
        const bool isHover = slider.isMouseOverOrDragging();

        // ─── Tick marks around the rim ────────────────────────────────────
        // Stepped params get one tick per step (capped); continuous get 11.
        int numTicks = 11;
        if (slider.getInterval() > 0.0)
        {
            const double range = slider.getMaximum() - slider.getMinimum();
            const int steps = (int) std::round (range / slider.getInterval()) + 1;
            if (steps >= 2 && steps <= 17) numTicks = steps;
        }
        const float tickInner = radius - strokeWidth * 0.4f;
        const float tickOuter = radius + 1.5f;
        const bool bipolar = slider.getMinimum() < -0.001 && slider.getMaximum() > 0.001;
        for (int t = 0; t < numTicks; ++t)
        {
            const float frac = (float) t / (float) (numTicks - 1);
            const float a = rotaryStart + frac * (rotaryEnd - rotaryStart);
            const float dx = std::sin (a);
            const float dy = -std::cos (a);
            // Centre tick on bipolar gets a brighter accent
            const bool isCentreTick = bipolar && std::abs (frac - 0.5f) < 0.001f;
            g.setColour (isCentreTick ? Palette::secondary.withAlpha (0.7f)
                                      : Palette::outline.withAlpha (0.55f));
            g.drawLine (centre.x + dx * tickInner, centre.y + dy * tickInner,
                        centre.x + dx * tickOuter, centre.y + dy * tickOuter,
                        isCentreTick ? 1.4f : 0.9f);
        }

        // ─── Track arc (dim, full sweep) ───────────────────────────────────
        Path track;
        track.addCentredArc (centre.x, centre.y, radius - strokeWidth, radius - strokeWidth,
                             0.0f, rotaryStart, rotaryEnd, true);
        g.setColour (Palette::outline);
        g.strokePath (track, PathStrokeType (strokeWidth, PathStrokeType::curved, PathStrokeType::rounded));

        // ─── Fill arc — bipolar from centre, otherwise from start ─────────
        Path fill;
        if (bipolar)
        {
            const float midAngle = rotaryStart + (rotaryEnd - rotaryStart) * 0.5f;
            fill.addCentredArc (centre.x, centre.y, radius - strokeWidth, radius - strokeWidth,
                                0.0f, jmin (midAngle, angle), jmax (midAngle, angle), true);
        }
        else
        {
            fill.addCentredArc (centre.x, centre.y, radius - strokeWidth, radius - strokeWidth,
                                0.0f, rotaryStart, angle, true);
        }
        const auto fillColour = slider.isEnabled() ? Palette::primary : Palette::primaryDim;
        // Hover: thicker fill stroke + a soft outer glow
        if (isHover && slider.isEnabled())
        {
            g.setColour (Palette::hot.withAlpha (0.25f));
            g.strokePath (fill, PathStrokeType (strokeWidth + 4.0f, PathStrokeType::curved, PathStrokeType::rounded));
        }
        g.setColour (fillColour);
        g.strokePath (fill, PathStrokeType (isHover ? strokeWidth + 0.8f : strokeWidth,
                                            PathStrokeType::curved, PathStrokeType::rounded));

        // ─── Inner cap — radial gradient, tactile feel ────────────────────
        const float innerR = radius - strokeWidth * 2.4f;
        if (innerR > 3.0f)
        {
            ColourGradient capGrad (Palette::surfaceHi.brighter (0.05f),
                                    centre.x - innerR * 0.35f, centre.y - innerR * 0.35f,
                                    Palette::surface.darker (0.25f),
                                    centre.x + innerR, centre.y + innerR,
                                    true);
            g.setGradientFill (capGrad);
            g.fillEllipse (centre.x - innerR, centre.y - innerR, innerR * 2.0f, innerR * 2.0f);

            // Subtle inner bezel ring
            g.setColour (Palette::outlineDim.withAlpha (0.6f));
            g.drawEllipse (centre.x - innerR, centre.y - innerR, innerR * 2.0f, innerR * 2.0f, 0.8f);
        }

        // ─── Pointer ──────────────────────────────────────────────────────
        Path pointer;
        const float pointerLength = innerR * 0.78f;
        const float pointerThick  = jmax (2.5f, radius * 0.10f);
        pointer.addRoundedRectangle (-pointerThick * 0.5f, -innerR + 2.0f,
                                     pointerThick, pointerLength,
                                     pointerThick * 0.5f);
        g.setColour (Palette::secondary);
        g.fillPath (pointer, AffineTransform::rotation (angle).translated (centre));

        // Bright dot at the indicator tip — gives the knob a "live" feel
        const float tipR = jmax (1.5f, pointerThick * 0.7f);
        const float tipX = centre.x + std::sin (angle) * (innerR - 4.0f);
        const float tipY = centre.y - std::cos (angle) * (innerR - 4.0f);
        g.setColour (slider.isEnabled() ? Palette::hot : Palette::primaryDim);
        g.fillEllipse (tipX - tipR, tipY - tipR, tipR * 2.0f, tipR * 2.0f);

        // ─── Pivot ────────────────────────────────────────────────────────
        g.setColour (Palette::outline);
        g.fillEllipse (centre.x - 2.5f, centre.y - 2.5f, 5.0f, 5.0f);
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style,
                           juce::Slider& slider) override
    {
        using namespace juce;
        const auto bounds = Rectangle<int> (x, y, width, height).toFloat();

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            g.setColour (Palette::surface);
            g.fillRoundedRectangle (bounds, 4.0f);
            auto filled = bounds;
            if (style == Slider::LinearBar)
                filled.setWidth (sliderPos - bounds.getX());
            else
                filled.setTop (sliderPos);
            g.setColour (slider.isEnabled() ? Palette::primary : Palette::primaryDim);
            g.fillRoundedRectangle (filled, 4.0f);
            return;
        }

        if (style == Slider::LinearVertical)
        {
            const float trackX = bounds.getCentreX() - 2.0f;
            g.setColour (Palette::outlineDim);
            g.fillRoundedRectangle (trackX, bounds.getY(), 4.0f, bounds.getHeight(), 2.0f);

            const bool bipolar = slider.getMinimum() < -0.001 && slider.getMaximum() > 0.001;
            const float midY = bipolar ? bounds.getCentreY() : bounds.getBottom();
            g.setColour (slider.isEnabled() ? Palette::primary : Palette::primaryDim);
            g.fillRoundedRectangle (trackX,
                                    juce::jmin (sliderPos, midY),
                                    4.0f,
                                    std::abs (sliderPos - midY),
                                    2.0f);

            // Thumb
            g.setColour (Palette::secondary);
            g.fillEllipse (bounds.getCentreX() - 6.0f, sliderPos - 6.0f, 12.0f, 12.0f);
            return;
        }

        // Default: horizontal slider
        const float trackY = bounds.getCentreY() - 2.0f;
        g.setColour (Palette::outlineDim);
        g.fillRoundedRectangle (bounds.getX(), trackY, bounds.getWidth(), 4.0f, 2.0f);

        const bool bipolar = slider.getMinimum() < -0.001 && slider.getMaximum() > 0.001;
        const float midX = bipolar ? bounds.getCentreX() : bounds.getX();
        g.setColour (slider.isEnabled() ? Palette::primary : Palette::primaryDim);
        g.fillRoundedRectangle (juce::jmin (sliderPos, midX), trackY,
                                std::abs (sliderPos - midX), 4.0f, 2.0f);

        g.setColour (Palette::secondary);
        g.fillEllipse (sliderPos - 6.0f, bounds.getCentreY() - 6.0f, 12.0f, 12.0f);

        juce::ignoreUnused (minSliderPos, maxSliderPos);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool /*shouldDrawHighlighted*/, bool /*shouldDrawDown*/) override
    {
        using namespace juce;
        const auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
        const float boxW = juce::jmin (34.0f, bounds.getWidth() * 0.5f);
        const float boxH = juce::jmin (18.0f, bounds.getHeight() - 4.0f);
        if (boxW < 8.0f || boxH < 8.0f) return;

        auto box = Rectangle<float> (bounds.getX(), bounds.getCentreY() - boxH * 0.5f, boxW, boxH);
        const bool on = button.getToggleState();

        g.setColour (on ? Palette::primaryDim : Palette::outlineDim);
        g.fillRoundedRectangle (box, boxH * 0.5f);

        const float knobR = boxH - 4.0f;
        const float knobX = on ? box.getRight() - knobR - 2.0f : box.getX() + 2.0f;
        g.setColour (on ? Palette::secondary : Palette::textSecondary);
        g.fillEllipse (knobX, box.getY() + 2.0f, knobR, knobR);

        const auto textBounds = bounds.withTrimmedLeft (boxW + 8.0f);
        if (textBounds.getWidth() > 4.0f)
        {
            g.setColour (Palette::textPrimary);
            g.setFont (labelFont (13.0f));
            g.drawFittedText (button.getButtonText(), textBounds.toNearestInt(),
                              Justification::centredLeft, 1);
        }
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool /*isDown*/,
                       int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                       juce::ComboBox& box) override
    {
        using namespace juce;
        const auto bounds = Rectangle<int> (0, 0, width, height).toFloat().reduced (1.0f);
        g.setColour (Palette::surfaceHi);
        g.fillRoundedRectangle (bounds, 6.0f);

        g.setColour (box.hasKeyboardFocus (false) ? Palette::hot : Palette::outline);
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

        const float arrowSize = 6.0f;
        const float arrowX = bounds.getRight() - arrowSize - 10.0f;
        const float arrowY = bounds.getCentreY();
        Path arrow;
        arrow.addTriangle (arrowX, arrowY - arrowSize * 0.4f,
                           arrowX + arrowSize, arrowY - arrowSize * 0.4f,
                           arrowX + arrowSize * 0.5f, arrowY + arrowSize * 0.5f);
        g.setColour (Palette::primary);
        g.fillPath (arrow);
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (10, 1, box.getWidth() - 24, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return labelFont (13.0f); }
    juce::Font getLabelFont    (juce::Label&)    override { return labelFont (13.0f); }
    juce::Font getPopupMenuFont()                override { return labelFont (13.0f); }

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        g.fillAll (Palette::surface);
        g.setColour (Palette::outline);
        g.drawRect (0, 0, width, height, 1);
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool /*hasSubMenu*/, const juce::String& text,
                            const juce::String& /*shortcut*/, const juce::Drawable* /*icon*/,
                            const juce::Colour* /*textColour*/) override
    {
        using namespace juce;
        if (isSeparator)
        {
            g.setColour (Palette::outline);
            g.fillRect (area.reduced (6, 0).removeFromBottom (1));
            return;
        }
        if (isHighlighted && isActive)
        {
            g.setColour (Palette::primaryDim);
            g.fillRect (area);
        }

        const auto colour = isActive ? Palette::textPrimary : Palette::textDim;
        g.setColour (colour);
        g.setFont (getPopupMenuFont());

        auto textArea = area.reduced (10, 0);
        if (isTicked)
        {
            g.setColour (Palette::secondary);
            const auto tickArea = textArea.removeFromLeft (14);
            const auto r = tickArea.toFloat().withSizeKeepingCentre (8.0f, 8.0f);
            g.fillEllipse (r);
            g.setColour (colour);
        }
        else
        {
            textArea.removeFromLeft (14);
        }
        g.drawFittedText (text, textArea, Justification::centredLeft, 1);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawHighlighted, bool shouldDrawDown) override
    {
        using namespace juce;
        const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);

        auto base = backgroundColour;
        if (shouldDrawDown)
            base = base.darker (0.3f);
        else if (shouldDrawHighlighted)
            base = base.brighter (0.1f);

        g.setColour (base);
        g.fillRoundedRectangle (bounds, 4.0f);

        g.setColour (Palette::outline);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }
};

} // namespace cart::ui
