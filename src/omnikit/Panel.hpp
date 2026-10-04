#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/Scope.hpp"
#include <imgui.h>
#include <optional>

namespace OmniKit {

enum class PanelScroll {
    Auto,    // Scrollbar appears on overflow
    Always   // Vertical scrollbar is always visible
};

// Panel options (designated initializers):
//
//     if (Panel panel({.cardBackground = true}); panel) { ... }
//     if (Panel log({.key = "log", .scroll = PanelScroll::Always, .stickToEnd = autoScroll}); log) { ... }
struct PanelOptions {
    const char*           key            = nullptr;               // Identity; nullptr uses "##panel" (needed if multiple panels share a region)
    bool                  border         = true;
    bool                  cardBackground = false;                 // Card background instead of window background
    PanelScroll           scroll         = PanelScroll::Auto;
    bool                  stickToEnd     = false;                 // Keep scroll at bottom unless user scrolled up (logs)
    std::optional<ImVec2> sizePx;                                 // Final size, px; default fills all remaining space
};

// ============================================================================
// Panel — a scrollable region with a frame and (optional) card background.
// Wrapper over an ImGui child window.
// ============================================================================
class Panel : public Scope {
public:
    explicit Panel(const PanelOptions& options = {});
    ~Panel();

    // Remaining space inside panel, px (for nested elements requiring explicit size)
    ImVec2 Available() const { return ImGui::GetContentRegionAvail(); }

private:
    bool m_background = false;
    bool m_stickToEnd = false;
};

} // namespace OmniKit
