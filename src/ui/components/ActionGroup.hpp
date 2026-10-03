#pragma once

#include "ui/components/Button.hpp"
#include "ui/components/ToolButton.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// Контейнер группы действий в строке таблицы (автоматический UiSize::Mini, изоляция ID и отступы)
class ActionGroup {
public:
    explicit ActionGroup(int id, float topOffset = -1.0f)
        : m_pushedId(true)
    {
        const UiTheme& theme = UiTheme::Get();
        float offset = (topOffset >= 0.0f) ? topOffset : std::max(0.0f, (theme.table.rowHeight - 26.0f) * 0.5f - theme.table.cellPaddingY);
        if (offset > 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(offset));
        }
        ImGui::PushID(id);
    }

    ~ActionGroup() {
        if (m_pushedId) {
            ImGui::PopID();
        }
    }

    ActionGroup(const ActionGroup&) = delete;
    ActionGroup& operator=(const ActionGroup&) = delete;

    bool Button(const char* label, Icon icon = Icon::None, UiVariant variant = UiVariant::Default, float baseWidth = 0.0f) {
        BeforeNextItem();
        return ::Button({.label = label, .variant = variant, .icon = icon, .size = UiSize::Mini, .width = baseWidth});
    }

    bool ToolButton(Icon icon, const char* tooltip = nullptr, UiVariant variant = UiVariant::Default) {
        BeforeNextItem();
        return ::ToolButton({.icon = icon, .variant = variant, .size = UiSize::Mini, .tooltip = tooltip});
    }

private:
    void BeforeNextItem() {
        if (m_itemCount > 0) {
            const UiTheme& theme = UiTheme::Get();
            ImGui::SameLine(0.0f, theme.Scale(6.0f));
        }
        m_itemCount++;
    }

    bool m_pushedId = false;
    int m_itemCount = 0;
};
