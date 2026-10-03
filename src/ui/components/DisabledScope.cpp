#include "ui/components/DisabledScope.hpp"

#include <imgui_internal.h>

DisabledScope::DisabledScope(bool disabled) {
    ImGui::BeginDisabled(disabled);
    m_open = true;
    m_activeAliveBefore = ImGui::GetCurrentContext()->ActiveIdIsAlive;
}

DisabledScope::~DisabledScope() {
    const ImGuiContext& g = *ImGui::GetCurrentContext();
    // Активный элемент отрисован в этом кадре внутри выключенной области
    const bool activeInside = g.ActiveId != 0 && g.ActiveIdIsAlive == g.ActiveId &&
                              m_activeAliveBefore != g.ActiveId;
    if (Active() && activeInside) {
        ImGui::ClearActiveID();
    }
    ImGui::EndDisabled();
}

bool DisabledScope::Active() {
    return (ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
}
