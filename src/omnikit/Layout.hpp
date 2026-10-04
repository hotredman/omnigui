#pragma once

#include "omnikit/UiTheme.hpp"
#include <imgui.h>

namespace OmniKit {

// ============================================================================
// Layout primitives. All sizes are in base px (theme scaling is applied
// internally), so application code needs neither ImGui:: nor theme.Scale().
//
//     Text("Run #01");
//     SameLine();
//     Badge("COMPLETED", {.variant = UiVariant::Success});
//     Spacer();                  // standard vertical spacing
//     Spacer(12);                // 12 base px
//     AlignRight(240);           // next element at the right edge, 240 px for the group
//     Button({.label = "Reset"});
// ============================================================================

// Vertical spacing. basePx <= 0 — standard item spacing
inline void Spacer(float basePx = 0.0f) {
    if (basePx <= 0.0f) {
        ImGui::Spacing();
    } else {
        ImGui::Dummy(ImVec2(0.0f, UiTheme::Get().Scale(basePx)));
    }
}

// Next element on the same line. gapBasePx < 0 — standard gap
inline void SameLine(float gapBasePx = -1.0f) {
    if (gapBasePx < 0.0f) {
        ImGui::SameLine();
    } else {
        ImGui::SameLine(0.0f, UiTheme::Get().Scale(gapBasePx));
    }
}

// Next element on the same line, aligned to the right edge of the region:
// widthBasePx is the total width of the element group aligned to the right
inline void AlignRight(float widthBasePx) {
    ImGui::SameLine(ImGui::GetContentRegionMax().x - UiTheme::Get().Scale(widthBasePx));
}

} // namespace OmniKit
