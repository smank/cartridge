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
        const auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (6.0f);
        const float radius = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        if (radius < 4.0f) return;

        const Point<float> centre = bounds.getCentre();
        const float strokeWidth = Metrics::knobStrokeWidth;
        const float angle = rotaryStart + sliderPos * (rotaryEnd - rotaryStart);

        // Track arc (full sweep, dim)
        Path track;
        track.addCentredArc (centre.x, centre.y, radius - strokeWidth, radius - strokeWidth,
                             0.0f, rotaryStart, rotaryEnd, true);
        g.setColour (Palette::outline);
        g.strokePath (track, PathStrokeType (strokeWidth, PathStrokeType::curved, PathStrokeType::rounded));

        // Fill arc — bipolar from centre for ± params, otherwise from start
        Path fill;
        const bool bipolar = slider.getMinimum() < -0.001 && slider.getMaximum() > 0.001;
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
        g.setColour (slider.isEnabled() ? Palette::primary : Palette::primaryDim);
        g.strokePath (fill, PathStrokeType (strokeWidth, PathStrokeType::curved, PathStrokeType::rounded));

        // Inner soft fill — gives the cap a tactile feel
        const float innerR = radius - strokeWidth * 2.5f;
        if (innerR > 2.0f)
        {
            g.setColour (Palette::surface);
            g.fillEllipse (centre.x - innerR, centre.y - innerR, innerR * 2.0f, innerR * 2.0f);
        }

        // Pointer
        Path pointer;
        const float pointerLength = innerR * 0.75f;
        const float pointerThick  = 3.0f;
        pointer.addRectangle (-pointerThick * 0.5f, -innerR + 3.0f, pointerThick, pointerLength);
        g.setColour (Palette::secondary);
        g.fillPath (pointer, AffineTransform::rotation (angle).translated (centre));

        // Pivot dot
        g.setColour (Palette::outline);
        g.fillEllipse (centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
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
