#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include "omnikit/UiKey.hpp"
#include <imgui.h>
#include <optional>
#include <source_location>

namespace OmniKit {

// Square flat toolbar button: icon or short text
// glyph ("Aa", "+") with selected state. Parameters are designated initializers:
//
//     ToolButton({.icon = Icon::Cog, .tooltip = "Settings"});
//     ToolButton({.glyph = "Aa", .tooltip = "Font", .selected = isOn});
//
// Identity is inferred from key, otherwise from call site (loc), glyph, or icon.
struct ToolButtonOptions {
    Icon        icon     = Icon::None;
    const char* glyph    = nullptr;                 // text instead of icon
    UiVariant   variant  = UiVariant::Default;
    std::optional<UiSize> size;                     // square side; unset — theme size
    const char* tooltip  = nullptr;
    bool        selected = false;
    bool        disabled = false;
    ImU32       iconColor = 0;                      // 0 — color by state (icon only)
    UiKey       key      = {};                      // explicit identity
    float       sidePx   = 0.0f;                    // for components with precomputed geometry: side in final px, overrides size
};

// Returns true in the frame of click
bool ToolButton(const ToolButtonOptions& options,
                std::source_location loc = std::source_location::current());

// Vertical alignment of an item with itemHeightPx centered in bar (Header, etc.).
// Operates only in windows matching header height; does nothing in standard windows.
// allowMoveUp = true allows moving cursor upward (needed after SameLine when
// cursor inherited Y of previous shorter element)
void ApplyToolbarVerticalCentering(float itemHeightPx, bool allowMoveUp = false);

} // namespace OmniKit
