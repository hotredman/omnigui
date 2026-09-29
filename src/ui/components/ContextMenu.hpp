#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>
#include <imgui.h>

class ContextMenu {
public:
    explicit ContextMenu(std::string id, const ContextMenuStyle& style = UiTheme::Get().contextMenu);
    ContextMenu(std::string id, ImVec2 pos, const ContextMenuStyle& style = UiTheme::Get().contextMenu);
    ~ContextMenu();

    ContextMenu(const ContextMenu&) = delete;
    ContextMenu& operator=(const ContextMenu&) = delete;

    // Статическое открытие / проверка состояния меню по ID
    static void Open(const std::string& id);
    static bool IsOpen(const std::string& id);

    // RAII / функциональный вход в меню
    bool Begin();
    bool Begin(ImVec2 pos);
    void End();

    // Перегрузка bool для удобного синтаксиса: if (ContextMenu menu(id, pos); menu) { ... }
    explicit operator bool() const { return m_isOpen; }

    // Компоненты меню
    void Header(const std::string& text);
    bool Item(const std::string& label, bool selected = false, bool enabled = true);
    void Separator();

private:
    std::string m_id;
    ContextMenuStyle m_style;
    bool m_hasPos = false;
    ImVec2 m_pos{0.0f, 0.0f};
    bool m_isOpen = false;
    bool m_ended = false;
    int m_itemCounter = 0;
};
