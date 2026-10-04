#pragma once

#include "omnikit/Panel.hpp"
#include "omnikit/SidePanel.hpp"
#include "omnikit/UiTheme.hpp"
#include <algorithm>
#include <string>
#include <imgui.h>

namespace OmniKit {

// Split view options (designated initializers):
//
//     SplitView split({.sideWidth = 260});
//     if (auto main = split.Main({.cardBackground = true})) { ... }
//     if (auto side = split.Side()) { ... }
struct SplitViewOptions {
    float       sideWidth = 320.0f;   // Side panel width, base px
    const char* key       = nullptr;  // Split identity
};

// ============================================================================
// Main region + right side panel across the full remaining height.
// Computes widths automatically without manual GetContentRegionAvail math.
// Main() and Side() must be called once each and strictly in order.
// ============================================================================
class SplitView {
public:
    explicit SplitView(const SplitViewOptions& options = {})
        : m_sideWidth(options.sideWidth)
        , m_avail(ImGui::GetContentRegionAvail())
        , m_mainKey(options.key ? std::string(options.key) + "_main" : "##split_main")
        , m_sideKey(options.key ? std::string(options.key) + "_side" : "##split_side") {}

    // Main region: all space to the left of the side panel
    Panel Main(PanelOptions options = {}) const {
        const float sideW = SidePanel::CalcTotalWidth(m_sideWidth, false);
        const float mainW = std::max(50.0f, m_avail.x - sideW - UiTheme::Get().SpacingMedium());
        if (!options.sizePx) options.sizePx = ImVec2(mainW, m_avail.y);
        if (!options.key) options.key = m_mainKey.c_str();
        return Panel(options);
    }

    // Side panel to the right of the main region
    SidePanel Side() const {
        ImGui::SameLine();
        return SidePanel({.width = m_sideWidth, .key = m_sideKey.c_str()});
    }

private:
    float       m_sideWidth;
    ImVec2      m_avail;
    std::string m_mainKey;
    std::string m_sideKey;
};

} // namespace OmniKit
