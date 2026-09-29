#include "ui/components/SearchInput.hpp"
#include "ui/components/ToolButton.hpp"
#include <imgui.h>
#include <algorithm>

bool SearchInput::Render(const char* id, std::string& query, const char* hint, float baseWidth, UiSize size) {
    const UiTheme& theme = UiTheme::Get();
    ControlMetrics m = theme.GetMetrics(size);

    float totalW = 0.0f;
    float availW = ImGui::GetContentRegionAvail().x;
    if (baseWidth > 0.0f) {
        totalW = (availW > 0.0f) ? std::min(theme.Scale(baseWidth), availW) : theme.Scale(baseWidth);
    } else {
        totalW = (availW > 0.0f) ? availW : theme.Scale(280.0f);
    }
    float h = m.height;

    bool hasClearBtn = !query.empty();
    float clearBtnW = hasClearBtn ? h : 0.0f;
    float inputW = totalW - (hasClearBtn ? (clearBtnW + theme.Scale(4.0f)) : 0.0f);

    ImGui::PushID(id);

    float fontSize = theme.Scale(m.fontSize);
    float padY = std::max(2.0f, (m.height - fontSize) * 0.5f);

    // 1. Поле ввода с закруглением и фоном темы
    ImGui::PushItemWidth(inputW);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m.rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(m.paddingX, padY));

    theme.PushFont(m.font, m.fontSize);

    char buf[256];
    strncpy_s(buf, sizeof(buf), query.c_str(), _TRUNCATE);

    bool changed = false;
    if (ImGui::InputTextWithHint("##input", hint ? hint : "Search...", buf, sizeof(buf))) {
        query = buf;
        changed = true;
    }

    theme.PopFont();

    ImGui::PopStyleVar(2);
    ImGui::PopItemWidth();

    // 2. Clear query toolbutton
    if (hasClearBtn) {
        ImGui::SameLine(0.0f, theme.Scale(4.0f));
        if (ToolButton::Render("##clear", Icon::Close, size, UiVariant::Default, "Clear search")) {
            query.clear();
            changed = true;
        }
    }

    ImGui::PopID();
    return changed;
}
