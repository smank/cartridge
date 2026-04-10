#include "StepSequencerComponent.h"
#include "../PluginProcessor.h"

namespace cart {

namespace
{
    const juce::String channelNames[] = { "P1", "P2", "TRI", "NOI", "DPCM", "V.P1", "V.P2", "V.SAW" };
    const juce::String laneNames[] = { "VOL", "PITCH", "DUTY" };
}

StepSequencerComponent::StepSequencerComponent (CartridgeProcessor& proc)
    : processor (proc)
{
    // Channel buttons
    for (int i = 0; i < 8; ++i)
    {
        channelButtons[i].setButtonText (channelNames[i]);
        channelButtons[i].setClickingTogglesState (false);
        channelButtons[i].setColour (juce::TextButton::buttonColourId, Colors::bgMid);
        channelButtons[i].setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
        channelButtons[i].setColour (juce::TextButton::buttonOnColourId, Colors::accentActive);
        channelButtons[i].setColour (juce::TextButton::textColourOnId, Colors::textPrimary);
        channelButtons[i].onClick = [this, i]
        {
            selectedChannel = i;
            for (int j = 0; j < 8; ++j)
                channelButtons[j].setToggleState (j == i, juce::dontSendNotification);
            repaint();
        };
        addAndMakeVisible (channelButtons[i]);
    }
    channelButtons[0].setToggleState (true, juce::dontSendNotification);

    // Lane buttons
    for (int i = 0; i < 3; ++i)
    {
        laneButtons[i].setButtonText (laneNames[i]);
        laneButtons[i].setClickingTogglesState (false);
        laneButtons[i].setColour (juce::TextButton::buttonColourId, Colors::bgMid);
        laneButtons[i].setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
        laneButtons[i].setColour (juce::TextButton::buttonOnColourId, Colors::accentActive);
        laneButtons[i].setColour (juce::TextButton::textColourOnId, Colors::textPrimary);
        laneButtons[i].onClick = [this, i]
        {
            selectedLane = i;
            for (int j = 0; j < 3; ++j)
                laneButtons[j].setToggleState (j == i, juce::dontSendNotification);
            repaint();
        };
        addAndMakeVisible (laneButtons[i]);
    }
    laneButtons[0].setToggleState (true, juce::dontSendNotification);

    // Loop toggle
    loopToggle.setButtonText ("Loop");
    loopToggle.setColour (juce::ToggleButton::textColourId, Colors::textSecondary);
    loopToggle.setColour (juce::ToggleButton::tickColourId, Colors::accentActive);
    loopToggle.setToggleState (false, juce::dontSendNotification);
    loopToggle.onClick = [this] { setLooping (loopToggle.getToggleState()); };
    addAndMakeVisible (loopToggle);

    // Clear button
    clearButton.setButtonText ("Clr");
    clearButton.setColour (juce::TextButton::buttonColourId, Colors::bgMid);
    clearButton.setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
    clearButton.setTooltip ("Clear all steps for this lane");
    clearButton.onClick = [this] { clearAllSteps(); };
    addAndMakeVisible (clearButton);

    // Step count: minus button, display, plus button
    stepCountMinus.setButtonText ("-");
    stepCountMinus.setColour (juce::TextButton::buttonColourId, Colors::bgMid);
    stepCountMinus.setColour (juce::TextButton::textColourOffId, Colors::textPrimary);
    stepCountMinus.onClick = [this] { setStepCount (getStepCount() - 1); };
    addAndMakeVisible (stepCountMinus);

    stepCountPlus.setButtonText ("+");
    stepCountPlus.setColour (juce::TextButton::buttonColourId, Colors::bgMid);
    stepCountPlus.setColour (juce::TextButton::textColourOffId, Colors::textPrimary);
    stepCountPlus.onClick = [this] { setStepCount (getStepCount() + 1); };
    addAndMakeVisible (stepCountPlus);

    stepCountDisplay.setFont (juce::FontOptions (11.0f));
    stepCountDisplay.setColour (juce::Label::textColourId, Colors::textPrimary);
    stepCountDisplay.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (stepCountDisplay);

    startTimerHz (15);
}

StepSequencerComponent::~StepSequencerComponent()
{
    stopTimer();
}

void StepSequencerComponent::timerCallback()
{
    // Sync UI state from processor data
    loopToggle.setToggleState (isLooping(), juce::dontSendNotification);
    stepCountDisplay.setText (juce::String (getStepCount()), juce::dontSendNotification);

    // Only repaint if playback position changed
    int playStep = processor.seqPlaybackStep[selectedChannel].load (std::memory_order_relaxed);
    if (playStep != lastPlayStep)
    {
        lastPlayStep = playStep;
        repaint();
    }
}

void StepSequencerComponent::clearAllSteps()
{
    auto& sd = processor.getSequenceData (selectedChannel);
    int defaultVal = (selectedLane == LANE_VOLUME) ? 15 : 0;

    switch (selectedLane)
    {
        case LANE_VOLUME:
            for (int i = 0; i < StepSequenceData::kMaxSteps; ++i)
                sd.volumeSteps[i] = defaultVal;
            sd.numVolumeSteps = 0;
            break;
        case LANE_PITCH:
            for (int i = 0; i < StepSequenceData::kMaxSteps; ++i)
                sd.pitchSteps[i] = 0;
            sd.numPitchSteps = 0;
            break;
        case LANE_DUTY:
            for (int i = 0; i < StepSequenceData::kMaxSteps; ++i)
                sd.dutySteps[i] = 0;
            sd.numDutySteps = 0;
            break;
        default: break;
    }
    notifyDataChanged();
    repaint();
}

int StepSequencerComponent::getStepCount() const
{
    auto& sd = processor.getSequenceData (selectedChannel);
    switch (selectedLane)
    {
        case LANE_VOLUME: return sd.numVolumeSteps;
        case LANE_PITCH:  return sd.numPitchSteps;
        case LANE_DUTY:   return sd.numDutySteps;
        default: return 0;
    }
}

int StepSequencerComponent::getStepValue (int step) const
{
    auto& sd = processor.getSequenceData (selectedChannel);
    switch (selectedLane)
    {
        case LANE_VOLUME: return sd.volumeSteps[step];
        case LANE_PITCH:  return sd.pitchSteps[step];
        case LANE_DUTY:   return sd.dutySteps[step];
        default: return 0;
    }
}

void StepSequencerComponent::setStepValue (int step, int value)
{
    auto& sd = processor.getSequenceData (selectedChannel);
    switch (selectedLane)
    {
        case LANE_VOLUME: sd.volumeSteps[step] = value; break;
        case LANE_PITCH:  sd.pitchSteps[step] = value;  break;
        case LANE_DUTY:   sd.dutySteps[step] = value;   break;
        default: break;
    }
    notifyDataChanged();
}

int StepSequencerComponent::getMinValue() const
{
    switch (selectedLane)
    {
        case LANE_VOLUME: return 0;
        case LANE_PITCH:  return -12;
        case LANE_DUTY:   return 0;
        default: return 0;
    }
}

int StepSequencerComponent::getMaxValue() const
{
    switch (selectedLane)
    {
        case LANE_VOLUME: return 15;
        case LANE_PITCH:  return 12;
        case LANE_DUTY:
            return (selectedChannel == 5 || selectedChannel == 6) ? 7 : 3;
        default: return 15;
    }
}

bool StepSequencerComponent::isLooping() const
{
    auto& sd = processor.getSequenceData (selectedChannel);
    switch (selectedLane)
    {
        case LANE_VOLUME: return sd.volumeLoop;
        case LANE_PITCH:  return sd.pitchLoop;
        case LANE_DUTY:   return sd.dutyLoop;
        default: return false;
    }
}

void StepSequencerComponent::setLooping (bool loop)
{
    auto& sd = processor.getSequenceData (selectedChannel);
    switch (selectedLane)
    {
        case LANE_VOLUME: sd.volumeLoop = loop; break;
        case LANE_PITCH:  sd.pitchLoop = loop;  break;
        case LANE_DUTY:   sd.dutyLoop = loop;   break;
        default: break;
    }
    notifyDataChanged();
}

void StepSequencerComponent::setStepCount (int count)
{
    auto& sd = processor.getSequenceData (selectedChannel);
    count = juce::jlimit (0, StepSequenceData::kMaxSteps, count);
    switch (selectedLane)
    {
        case LANE_VOLUME: sd.numVolumeSteps = count; break;
        case LANE_PITCH:  sd.numPitchSteps = count;  break;
        case LANE_DUTY:   sd.numDutySteps = count;   break;
        default: break;
    }
    stepCountDisplay.setText (juce::String (count), juce::dontSendNotification);
    notifyDataChanged();
    repaint();
}

void StepSequencerComponent::notifyDataChanged()
{
    processor.sequenceDataVersion.fetch_add (1, std::memory_order_release);
}

juce::Rectangle<int> StepSequencerComponent::getGridArea() const
{
    return getLocalBounds().withTrimmedTop (24).withTrimmedBottom (2).reduced (4, 0);
}

juce::Rectangle<int> StepSequencerComponent::getCellRect (int step) const
{
    auto grid = getGridArea();
    int cellW = grid.getWidth() / StepSequenceData::kMaxSteps;
    return { grid.getX() + step * cellW, grid.getY(), cellW, grid.getHeight() };
}

void StepSequencerComponent::editStep (const juce::MouseEvent& e)
{
    auto grid = getGridArea();
    auto pos = e.getPosition();

    if (!grid.contains (pos))
        return;

    int cellW = grid.getWidth() / StepSequenceData::kMaxSteps;
    int step = (pos.getX() - grid.getX()) / cellW;
    int numSteps = getStepCount();

    if (step < 0 || step >= numSteps)
        return;

    int minVal = getMinValue();
    int maxVal = getMaxValue();
    int range = maxVal - minVal;
    if (range == 0) return;

    // Map Y position to value (top = max, bottom = min)
    float normalizedY = 1.0f - static_cast<float> (pos.getY() - grid.getY())
                              / static_cast<float> (grid.getHeight());
    int value = minVal + juce::roundToInt (normalizedY * static_cast<float> (range));
    value = juce::jlimit (minVal, maxVal, value);

    setStepValue (step, value);
    repaint();
}

void StepSequencerComponent::mouseDown (const juce::MouseEvent& e)
{
    // Right-click on a step: reset it to default
    if (e.mods.isRightButtonDown())
    {
        auto grid = getGridArea();
        auto pos = e.getPosition();
        if (grid.contains (pos))
        {
            int cellW = grid.getWidth() / StepSequenceData::kMaxSteps;
            int step = (pos.getX() - grid.getX()) / cellW;
            if (step >= 0 && step < getStepCount())
            {
                int defaultVal = (selectedLane == LANE_VOLUME) ? 15 : 0;
                setStepValue (step, defaultVal);
                repaint();
            }
        }
        return;
    }

    editStep (e);
}

void StepSequencerComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
        return;
    editStep (e);
}

void StepSequencerComponent::paint (juce::Graphics& g)
{
    auto grid = getGridArea();
    int numSteps = getStepCount();
    int minVal = getMinValue();
    int maxVal = getMaxValue();
    int range = maxVal - minVal;
    float gridH = static_cast<float> (grid.getHeight());

    // Grid background
    g.setColour (Colors::bgDark);
    g.fillRect (grid);

    // Beat group markers (every 4 steps)
    g.setColour (Colors::divider.withAlpha (0.15f));
    for (int i = 4; i < StepSequenceData::kMaxSteps; i += 4)
    {
        auto cell = getCellRect (i);
        g.fillRect (cell.getX() - 1, grid.getY(), 2, grid.getHeight());
    }

    // Center line for pitch lane
    if (selectedLane == LANE_PITCH)
    {
        int centerY = grid.getCentreY();
        g.setColour (Colors::textDark);
        g.drawHorizontalLine (centerY, (float) grid.getX(), (float) grid.getRight());
    }

    // Get playback position
    int playStep = processor.seqPlaybackStep[selectedChannel].load (std::memory_order_relaxed);
    bool channelPlaying = processor.channelActive[selectedChannel].load (std::memory_order_relaxed);

    for (int i = 0; i < StepSequenceData::kMaxSteps; ++i)
    {
        auto cell = getCellRect (i);
        bool active = (i < numSteps);

        // Grid lines
        g.setColour (Colors::divider.withAlpha (0.2f));
        g.drawVerticalLine (cell.getX(), (float) grid.getY(), (float) grid.getBottom());

        // Inactive steps — dimmed overlay
        if (!active)
        {
            g.setColour (Colors::bgMid.withAlpha (0.6f));
            g.fillRect (cell.reduced (1));
            continue;
        }

        // Playback highlight
        if (i == playStep && channelPlaying)
        {
            g.setColour (Colors::accentDim.withAlpha (0.5f));
            g.fillRect (cell.reduced (1));
        }

        // Draw value bar
        int value = getStepValue (i);

        if (range > 0)
        {
            auto inner = cell.reduced (2, 1);

            if (selectedLane == LANE_PITCH)
            {
                // Bipolar: bars grow from center
                int centerY = grid.getCentreY();
                float halfH = gridH * 0.5f;
                float barExtent = (static_cast<float> (std::abs (value)) / static_cast<float> (maxVal)) * halfH;
                int barH = juce::roundToInt (barExtent);

                juce::Rectangle<int> barRect;
                if (value > 0)
                    barRect = { inner.getX(), centerY - barH, inner.getWidth(), barH };
                else if (value < 0)
                    barRect = { inner.getX(), centerY, inner.getWidth(), barH };
                else
                    barRect = { inner.getX(), centerY, inner.getWidth(), 1 };

                g.setColour (value >= 0 ? Colors::accentActive : Colors::fxAccent);
                g.fillRect (barRect);
            }
            else
            {
                // Unipolar: bars grow from bottom
                float norm = static_cast<float> (value - minVal) / static_cast<float> (range);
                int barH = juce::jmax (1, juce::roundToInt (norm * static_cast<float> (inner.getHeight())));
                auto barRect = inner.withTop (inner.getBottom() - barH);

                g.setColour (Colors::accentActive);
                g.fillRect (barRect);
            }
        }

        // Value text — draw at top for unipolar, above/below bar for pitch
        g.setColour (Colors::textPrimary.withAlpha (0.8f));
        g.setFont (juce::FontOptions (9.0f));
        if (selectedLane == LANE_PITCH && value != 0)
        {
            juce::String prefix = value > 0 ? "+" : "";
            g.drawText (prefix + juce::String (value), cell.withHeight (12), juce::Justification::centred);
        }
        else
        {
            g.drawText (juce::String (value), cell.withHeight (12), juce::Justification::centred);
        }
    }

    // Outer border
    g.setColour (Colors::divider);
    g.drawRect (grid);

    // Loop indicator: small arrow from end back to start
    if (isLooping() && numSteps > 0)
    {
        auto lastCell = getCellRect (numSteps - 1);
        g.setColour (Colors::accentActive.withAlpha (0.6f));
        int y = grid.getBottom() + 1;
        g.drawArrow (juce::Line<float> ((float) lastCell.getCentreX(), (float) y,
                                         (float) getCellRect (0).getCentreX(), (float) y),
                      1.5f, 5.0f, 5.0f);
    }
}

void StepSequencerComponent::resized()
{
    auto bounds = getLocalBounds();
    auto topRow = bounds.removeFromTop (22);

    // Channel buttons
    int btnW = 36;
    for (int i = 0; i < 8; ++i)
        channelButtons[i].setBounds (topRow.removeFromLeft (btnW).reduced (1));

    topRow.removeFromLeft (6);

    // Lane buttons
    for (int i = 0; i < 3; ++i)
        laneButtons[i].setBounds (topRow.removeFromLeft (42).reduced (1));

    topRow.removeFromLeft (6);

    // Loop toggle
    loopToggle.setBounds (topRow.removeFromLeft (48));

    // Clear button
    clearButton.setBounds (topRow.removeFromLeft (28).reduced (1));

    topRow.removeFromLeft (4);

    // Step count: [-] [N] [+]
    stepCountMinus.setBounds (topRow.removeFromLeft (20).reduced (1));
    stepCountDisplay.setBounds (topRow.removeFromLeft (20));
    stepCountPlus.setBounds (topRow.removeFromLeft (20).reduced (1));
}

} // namespace cart
