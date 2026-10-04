#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <vector>
#include <type_traits>
#include <source_location>

namespace OmniKit {

// Combo box options (designated initializers):
//
//     Combo(mode, modes, 3, {.label = "Mode", .width = 200.0f});
//
// Identity is derived from key, or from call site (loc) and label.
struct ComboOptions {
    const char* label   = nullptr;              // label above combo box
    UiVariant   variant = UiVariant::Default;
    UiSize      size    = UiSize::Medium;
    float       width   = 0.0f;                 // base px (unscaled); 0 — fit content; Fill — stretch full width
    UiKey       key     = {};                   // explicit identity
    ImVec2      sizePx  = {};                   // for container components with computed geometry: resolved px, overrides width and size
    const ComboStyle* style = nullptr;          // nullptr — current theme style
};

// Returns true on the frame a new item is selected
bool Combo(int& currentItem, const char* const items[], int itemsCount, const ComboOptions& options = {},
           std::source_location loc = std::source_location::current());
bool Combo(int& currentItem, const std::vector<std::string>& items, const ComboOptions& options = {},
           std::source_location loc = std::source_location::current());

// Strongly-typed enum + array with deduced size
template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
bool Combo(EnumT& currentItem, const char* const (&items)[N], const ComboOptions& options = {},
           std::source_location loc = std::source_location::current()) {
    int current = static_cast<int>(currentItem);
    if (Combo(current, items, static_cast<int>(N), options, loc)) {
        currentItem = static_cast<EnumT>(current);
        return true;
    }
    return false;
}

} // namespace OmniKit
