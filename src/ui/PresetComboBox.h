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

    /// Set the current engine mode for filtering (0=Classic, 1=Modern)
    void setEngineMode (int mode) { currentEngineMode = mode; }

    /// Set engine mode per factory preset index (0=Classic, 1=Modern)
    void setPresetEngineModes (std::vector<int> modes) { presetEngineModes = std::move (modes); }

    /// Returns true if the factory preset at the given index matches the current engine mode
    bool isPresetVisibleForMode (int presetIndex) const
    {
        if (presetIndex < 0 || presetIndex >= static_cast<int> (presetEngineModes.size()))
            return true;  // User presets or out of range — always visible
        return presetEngineModes[static_cast<size_t> (presetIndex)] == currentEngineMode;
    }

    void showPopup() override
    {
        juce::PopupMenu menu;

        // Category ranges matching PresetManager::buildPresets() order
        // Classic presets: 0-29, Modern presets: 30-34
        struct Category { const char* name; int start; int end; bool columnBreak; int engineMode; };
        const Category categories[] = {
            { "INIT",              0,  0, false, 0 },
            { "LEADS",             1,  5, false, 0 },
            { "BASS",              6,  8, false, 0 },
            //--- column 2 ---
            { "PERCUSSION",        9, 12, true,  0 },
            { "SFX",              13, 15, false, 0 },
            { "FULL SETUPS",      16, 18, false, 0 },
            //--- column 3 ---
            { "WITH EFFECTS",     19, 20, true,  0 },
            { "ARP & SHOWCASE",   21, 24, false, 0 },
            //--- column 4 ---
            { "GAME INSPIRED",    25, 29, true,  0 },
            //--- Modern Engine ---
            { "MODERN ENGINE",    30, 34, true,  1 },
        };

        int numItems = getNumItems();
        int selectedId = getSelectedId();

        for (const auto& cat : categories)
        {
            if (cat.start >= numItems) break;

            // Skip entire categories that don't match current engine mode
            if (cat.engineMode != currentEngineMode)
                continue;

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

        // User presets — always visible in both modes, organized by category
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
    int currentEngineMode = 0;  // 0=Classic, 1=Modern
    std::vector<int> presetEngineModes;
    std::vector<UserCategory> userCategories;
};

} // namespace cart
