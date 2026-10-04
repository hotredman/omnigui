#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <cstddef>
#include <string>
#include <vector>
#include <utility>
#include <source_location>

namespace OmniKit {

// Tab bar options (designated initializers):
//
//     TabBar(tabs, selected);
//     TabBar(tabs, selected, {.separator = false, .variants = tabVariants});
struct TabBarOptions {
    UiSize             size      = UiSize::Medium;  // Tab height on the UiSize scale
    bool               separator = true;      // Separator line below tab bar
    const UiVariant*   variants  = nullptr;   // Per-tab variant; nullptr means all default
    UiKey              key       = {};        // Identity; default is label of first tab
    const TabBarStyle* style     = nullptr;   // Style override; nullptr uses theme
};

// 1. Tab bar from C-array of strings. Returns true if another tab was selected
bool TabBar(const char* const items[], int itemsCount, int& selectedIndex,
            const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current());

// 2. Tab bar from fixed-size array, size deduced automatically
template<std::size_t N>
bool TabBar(const char* const (&items)[N], int& selectedIndex, const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current()) {
    return TabBar(items, static_cast<int>(N), selectedIndex, options, loc);
}

// 3. Tab bar from vector of strings
bool TabBar(const std::vector<std::string>& items, int& selectedIndex,
            const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current());

// 4. For any enum type: tabs are (value, label) pairs
template<typename EnumT>
bool TabBar(const std::vector<std::pair<EnumT, std::string>>& items, EnumT& currentItem,
            const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current())
{
    int selectedIndex = -1;
    for (size_t i = 0; i < items.size(); ++i) {
        if (currentItem == items[i].first) {
            selectedIndex = static_cast<int>(i);
            break;
        }
    }
    std::vector<std::string> labels;
    labels.reserve(items.size());
    for (const auto& it : items) {
        labels.push_back(it.second);
    }
    bool changed = TabBar(labels, selectedIndex, options, loc);
    if (changed && selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size())) {
        currentItem = items[selectedIndex].first;
    }
    return changed;
}

struct TabItemOptions {
    UiSize             size     = UiSize::Medium;      // Tab height on the UiSize scale
    UiVariant          variant  = UiVariant::Default;  // Non-Default sets tab color and underline
    bool               sameLine = false;               // Continue on same line (with style gap)
    UiKey              key      = {};                  // Identity; default uses label
    const TabBarStyle* style    = nullptr;             // Style override; nullptr uses theme

    // Internal mechanism: final px; 0 for auto (from text / style height)
    ImVec2             sizePx   = ImVec2(0.0f, 0.0f);
};

// Single tab item (for custom layouts; TabBar is typically sufficient)
bool TabItem(const char* label, bool selected, const TabItemOptions& options = {},
             std::source_location loc = std::source_location::current());

// Horizontal separator line below tab bar
void TabSeparator(const TabBarStyle* style = nullptr);

} // namespace OmniKit
