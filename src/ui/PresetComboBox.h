#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cart {

class PresetComboBox : public juce::ComboBox
{
public:
    using juce::ComboBox::ComboBox;

    void setFactoryPresetCount (int count) { factoryPresetCount = count; }

    void showPopup() override
    {
        juce::PopupMenu menu;

        // Category ranges matching PresetManager::buildPresets() order
        struct Category { const char* name; int start; int end; bool columnBreak; };
        const Category categories[] = {
            { "INIT",              0,  0, false },
            { "LEADS",             1,  5, false },
            { "BASS",              6,  8, false },
            //--- column 2 ---
            { "PERCUSSION",        9, 12, true  },
            { "SFX",              13, 15, false },
            { "FULL SETUPS",      16, 18, false },
            //--- column 3 ---
            { "WITH EFFECTS",     19, 20, true  },
            { "ARP & SHOWCASE",   21, 24, false },
        };

        int numItems = getNumItems();
        int selectedId = getSelectedId();

        for (const auto& cat : categories)
        {
            if (cat.start >= numItems) break;

            if (cat.columnBreak)
                menu.addColumnBreak();

            menu.addSectionHeader (cat.name);

            int last = juce::jmin (cat.end, numItems - 1);
            for (int i = cat.start; i <= last; ++i)
            {
                int itemId = i + 1;
                menu.addItem (itemId, getItemText (i), true, itemId == selectedId);
            }
        }

        // User presets in rightmost column
        if (factoryPresetCount > 0 && numItems > factoryPresetCount)
        {
            menu.addColumnBreak();
            menu.addSectionHeader ("USER PRESETS");

            for (int i = factoryPresetCount; i < numItems; ++i)
            {
                int itemId = i + 1;
                menu.addItem (itemId, getItemText (i), true, itemId == selectedId);
            }
        }

        auto options = juce::PopupMenu::Options()
                           .withTargetComponent (this)
                           .withMinimumWidth (getWidth())
                           .withPreferredPopupDirection (juce::PopupMenu::Options::PopupDirection::downwards);

        menu.showMenuAsync (options, [safeThis = juce::Component::SafePointer<PresetComboBox> (this)] (int result)
        {
            if (safeThis == nullptr) return;
            safeThis->hidePopup();
            if (result > 0)
                safeThis->setSelectedId (result);
        });
    }

private:
    int factoryPresetCount = 0;
};

} // namespace cart
