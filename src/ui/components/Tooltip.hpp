#pragma once

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
