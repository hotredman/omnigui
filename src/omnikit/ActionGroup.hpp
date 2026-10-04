#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/Button.hpp"
#include "omnikit/ToolButton.hpp"
#include "omnikit/UiTheme.hpp"
#include <imgui.h>
#include <algorithm>

namespace OmniKit {

// Параметры группы действий
struct ActionGroupOptions {
    float topOffset = -1.0f;   // базовые px; <0 — выровнять по центру строки таблицы
};

// Группа действий в строке таблицы (автоматический UiSize::Mini и отступы).
// RAII-область без собственного ID: в циклах оборачивайте строки в IdScope.
//
//     IdScope row(i);
//     if (ActionGroup actions; actions) {
//         if (actions.Button({.label = "Edit"})) { ... }
//         if (actions.ToolButton({.icon = Icon::Trash, .tooltip = "Delete"})) { ... }
//     }
class ActionGroup : public Scope {
public:
    explicit ActionGroup(const ActionGroupOptions& options = {}) {
        const UiTheme& theme = UiTheme::Get();
        float offset = (options.topOffset >= 0.0f)
            ? options.topOffset
            : std::max(0.0f, (theme.table.rowHeight - 26.0f) * 0.5f - theme.table.cellPaddingY);
        if (offset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(offset));
        }
        m_open = true;
    }
    ~ActionGroup() = default;

    // size принудительно UiSize::Mini
    bool Button(ButtonOptions options) {
        BeforeNextItem();
        options.size = UiSize::Mini;
        return OmniKit::Button(options);
    }

    bool ToolButton(ToolButtonOptions options) {
        BeforeNextItem();
        options.size = UiSize::Mini;
        return OmniKit::ToolButton(options);
    }

private:
    void BeforeNextItem() {
        if (m_itemCount > 0) {
            const UiTheme& theme = UiTheme::Get();
            ImGui::SameLine(0.0f, theme.Scale(6.0f));
        }
        m_itemCount++;
    }

    int m_itemCount = 0;
};

} // namespace OmniKit
