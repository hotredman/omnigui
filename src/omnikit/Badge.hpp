#pragma once

#include "omnikit/UiTheme.hpp"

namespace OmniKit {

// Semantic badge/chip options (designated initializers):
//
//     Badge("RUNNING", {.variant = UiVariant::Success});
//     Badge("beta", {.variant = UiVariant::Warning, .tooltip = "Experimental"});
struct BadgeOptions {
    UiVariant   variant   = UiVariant::Default;
    UiSize      size      = UiSize::Medium;  // badge scale on the UiSize scale (Medium — theme style)
    const char* tooltip   = nullptr;     // hover tooltip
    float       topOffset = -1.0f;       // downward cursor offset, base px; < 0 — no offset
    float       fontSize  = 0.0f;        // font size, pt; 0 — derived from theme style
};

// Universal semantic chip/badge for displaying statuses, labels, and tags
void Badge(const char* text, const BadgeOptions& options = {});

// Numeric badge with a value and optional unit suffix (e.g., 5 "tests")
void BadgeNumber(int value, const char* unit = nullptr, const BadgeOptions& options = {});

// Direct rendering of a badge to the specified DrawList position (without advancing ImGui cursor);
// tooltip and topOffset are not used here. Returns the badge size.
ImVec2 BadgeDraw(ImDrawList* dl, ImVec2 pos, const char* text, const BadgeOptions& options = {});

// Calculates badge size
ImVec2 BadgeSize(const char* text, const BadgeOptions& options = {});

} // namespace OmniKit
