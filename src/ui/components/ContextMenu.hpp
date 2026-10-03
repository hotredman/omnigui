#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include <optional>
#include <string>
#include <imgui.h>

// Параметры контекстного меню (designated initializers):
//
//     if (ContextMenu menu(open, {.pos = ImVec2(x, y)}); menu) { ... }
struct ContextMenuOptions {
    std::optional<ImVec2> pos;                  // экранная позиция; по умолчанию — у курсора мыши
    const ContextMenuStyle* style = nullptr;    // оверрайд стиля; nullptr — из темы
};

// Параметры пункта меню
struct MenuItemOptions {
    bool selected = false;    // отмечен галочкой
    bool disabled = false;
};

// ============================================================================
// Контекстное меню (RAII-область). Состояние «открыто» хранит приложение (bool):
// выставьте его в true, чтобы показать меню; меню само сбросит флаг при выборе
// пункта и при закрытии кликом снаружи. Идентичность меню — адрес этого флага.
//
//     static bool s_menuOpen = false;
//     if (ToolButton({.icon = Icon::List})) s_menuOpen = true;
//     if (ContextMenu menu(s_menuOpen); menu) {
//         menu.Header("SCALE");
//         if (menu.Item("100%", {.selected = true})) { ... }
//     }
//
// Второй режим — контекстное меню последнего элемента (правый клик), флаг не нужен:
//
//     s_sidebarMenu.Item(...);
//     if (ContextMenu menu(ContextMenu::OnLastItem); menu) {
//         if (menu.Item("Delete")) { ... }
//     }
// ============================================================================
class ContextMenu : public Scope {
public:
    struct LastItemTag {};
    static constexpr LastItemTag OnLastItem{};

    explicit ContextMenu(bool& isOpen, const ContextMenuOptions& options = {});
    explicit ContextMenu(LastItemTag, const ContextMenuOptions& options = {});
    ~ContextMenu();

    // Компоненты меню
    void Header(const std::string& text);
    bool Item(const std::string& label, const MenuItemOptions& options = {});
    void Separator();

private:
    void PushStyle();
    void PopStyle();

    bool* m_openRef = nullptr;    // nullptr в режиме OnLastItem
    bool m_idPushed = false;
    ContextMenuStyle m_style;
    int m_itemCounter = 0;
};
