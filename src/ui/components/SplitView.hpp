#pragma once

#include "ui/components/Panel.hpp"
#include "ui/components/SidePanel.hpp"
#include "ui/components/UiTheme.hpp"
#include <algorithm>
#include <string>
#include <imgui.h>

// Параметры разделённой области (designated initializers):
//
//     SplitView split({.sideWidth = 260});
//     if (auto main = split.Main({.cardBackground = true})) { ... }
//     if (auto side = split.Side()) { ... }
struct SplitViewOptions {
    float       sideWidth = 320.0f;   // ширина боковой панели, базовые px
    const char* key       = nullptr;  // идентичность сплита
};

// ============================================================================
// Основная область + боковая панель справа во всю оставшуюся высоту.
// Считает ширины сам: прикладному коду не нужна арифметика GetContentRegionAvail.
// Main() и Side() вызываются по одному разу и строго по порядку.
// ============================================================================
class SplitView {
public:
    explicit SplitView(const SplitViewOptions& options = {})
        : m_sideWidth(options.sideWidth)
        , m_avail(ImGui::GetContentRegionAvail())
        , m_mainKey(options.key ? std::string(options.key) + "_main" : "##split_main")
        , m_sideKey(options.key ? std::string(options.key) + "_side" : "##split_side") {}

    // Основная область: всё место слева от боковой панели
    Panel Main(PanelOptions options = {}) const {
        const float sideW = SidePanel::CalcTotalWidth(m_sideWidth, false);
        const float mainW = std::max(50.0f, m_avail.x - sideW - UiTheme::Get().SpacingMedium());
        if (!options.sizePx) options.sizePx = ImVec2(mainW, m_avail.y);
        if (!options.key) options.key = m_mainKey.c_str();
        return Panel(options);
    }

    // Боковая панель справа от основной области
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
