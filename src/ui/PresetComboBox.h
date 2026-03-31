#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>

namespace cart {

class PresetComboBox : public juce::ComboBox
{
public:
    using juce::ComboBox::ComboBox;

    void setFactoryPresetCount (int count) { factoryPresetCount = count; }

    /// Set user category info for the popup menu.
    /// Each entry is { categoryName, [presetIndices...] }.
    /// Uncategorized user presets have an empty category name.
    struct UserCategory
    {
        juce::String name;
        std::vector<int> indices;  // preset indices (0-based)
    };

    void setUserCategories (std::vector<UserCategory> cats) { userCategories = std::move (cats); }

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
            //--- column 4 ---
            { "GAME INSPIRED",    25, 29, true  },
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

        // User presets — organized by category
        if (factoryPresetCount > 0 && numItems > factoryPresetCount)
        {
            bool needsColumnBreak = true;

            if (! userCategories.empty())
            {
                // Show categorized presets first
                for (const auto& cat : userCategories)
                {
                    if (cat.indices.empty()) continue;

                    if (needsColumnBreak)
                    {
                        menu.addColumnBreak();
                        needsColumnBreak = false;
                    }

                    juce::String header = cat.name.isEmpty() ? "USER PRESETS" : cat.name.toUpperCase();
                    menu.addSectionHeader (header);

                    for (int idx : cat.indices)
                    {
                        if (idx >= 0 && idx < numItems)
                        {
                            int itemId = idx + 1;
                            menu.addItem (itemId, getItemText (idx), true, itemId == selectedId);
                        }
                    }
                }
            }
            else
            {
                // Flat list fallback
                menu.addColumnBreak();
                menu.addSectionHeader ("USER PRESETS");

                for (int i = factoryPresetCount; i < numItems; ++i)
                {
                    int itemId = i + 1;
                    menu.addItem (itemId, getItemText (i), true, itemId == selectedId);
                }
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
    std::vector<UserCategory> userCategories;
};

} // namespace cart
