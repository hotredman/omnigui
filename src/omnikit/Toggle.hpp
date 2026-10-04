#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <source_location>

namespace OmniKit {

// =========================================================================================
// Toggle component (modern slider switch styled after @evo/ui/Toggle.svelte)
// =========================================================================================

// Toggle parameters (designated initializers):
//
//     Toggle(enabled, {.label = "Auto", .sublabel = "Adjusts on its own"});
//
// Identity is inferred from key, otherwise from call site (loc) and label.
struct ToggleOptions {
    const char* label    = nullptr;
    UiSize      size     = UiSize::Medium;    // scale by UiSize (Medium is theme style)
    const char* sublabel = nullptr;           // explanation below label
    float       width    = 0.0f;              // base px (before scale); 0 — content sized
    bool        disabled = false;
    UiKey       key      = {};                // explicit identity
    ImVec2      sizePx   = {};                // for containers with precomputed geometry: final px, overrides width
    const ToggleStyle* style = nullptr;       // nullptr — current theme style
};

// Toggles value on click; returns true in the frame of change
bool Toggle(bool& value, const ToggleOptions& options = {},
            std::source_location loc = std::source_location::current());

} // namespace OmniKit
