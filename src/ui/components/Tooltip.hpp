#pragma once

#include "ui/components/Scope.hpp"
#include <imgui.h>

// ============================================================================
// Подсказка у последнего нарисованного элемента. Показывается и у выключенного (кнопка
// с причиной недоступности). nullptr и пустой текст — подсказки нет.
// ============================================================================
namespace Tooltip {

inline void OnLastItem(const char* text) {
    if (text && text[0] != '\0' && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", text);
}

}  // namespace Tooltip

// Развёрнутая подсказка для последнего элемента (RAII-область): содержимое —
// любые компоненты. Для простого текста используйте Tooltip::OnLastItem.
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
