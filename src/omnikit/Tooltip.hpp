#pragma once

#include "omnikit/Scope.hpp"
#include <imgui.h>

namespace OmniKit {

// ============================================================================
// Tooltip for the last rendered item. Displayed also for disabled items (button
// showing reason for unavailability). nullptr and empty text — no tooltip.
// ============================================================================
namespace Tooltip {

inline void OnLastItem(const char* text) {
    if (text && text[0] != '\0' && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", text);
}

}  // namespace Tooltip

// Detailed tooltip for the last item (RAII scope): contents can be
// any components. For plain text use Tooltip::OnLastItem.
//
//     Button({.label = "Run"});
//     if (ItemTooltip tip; tip) {
//         Text("Started: 10:14");
//         Badge("COMPLETED", {.variant = UiVariant::Success});
//     }
class ItemTooltip : public Scope {
public:
    ItemTooltip() {
        if (ImGui::IsItemHovered()) m_open = ImGui::BeginTooltip();
    }
    ~ItemTooltip() {
        if (m_open) ImGui::EndTooltip();
    }
};

} // namespace OmniKit
