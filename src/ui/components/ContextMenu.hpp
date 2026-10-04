#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include <optional>
#include <string>
#include <imgui.h>

// Context menu options (designated initializers):
//
//     if (ContextMenu menu(open, {.pos = ImVec2(x, y)}); menu) { ... }
struct ContextMenuOptions {
    std::optional<ImVec2> pos;                  // Screen position; defaults to mouse cursor
    const ContextMenuStyle* style = nullptr;    // Style override; nullptr = from active UiTheme
};

// Context menu item options
struct MenuItemOptions {
    bool selected = false;    // Marked with checkmark
    bool disabled = false;
};

// ============================================================================
// ContextMenu (RAII Scope).
// Mode 1: Controlled by a boolean flag (e.g. triggered by a tool button):
//
//     static bool s_menuOpen = false;
//     if (ToolButton({.icon = Icon::List})) s_menuOpen = true;
//     if (ContextMenu menu(s_menuOpen); menu) {
//         menu.Header("SCALE");
//         if (menu.Item("100%", {.selected = true})) { ... }
//     }
//
// Mode 2: Triggered by right-clicking the previous ImGui item:
//
//     s_sidebarMenu.Item(...);
//     if (ContextMenu menu(ContextMenu::OnLastItem); menu) {
//         if (menu.Item("Delete")) { ... }
//     }
//
// Mode 3: Attached to an ImGui popup ID (e.g. opened via ImGui::OpenPopup):
//
//     if (ContextMenu menu("##my_ctx_popup"); menu) {
//         menu.Header("ACTIONS");
//         if (menu.Item("Reset")) { ... }
//     }
// ============================================================================
class ContextMenu : public Scope {
public:
    struct LastItemTag {};
    static constexpr LastItemTag OnLastItem{};

    explicit ContextMenu(bool& isOpen, const ContextMenuOptions& options = {});
    explicit ContextMenu(LastItemTag, const ContextMenuOptions& options = {});
    explicit ContextMenu(const char* popupId, const ContextMenuOptions& options = {});
    ~ContextMenu();

    // Menu components
    void Header(const char* text);
    void Header(const std::string& text) { Header(text.c_str()); }
    bool Item(const char* label, const MenuItemOptions& options = {});
    bool Item(const std::string& label, const MenuItemOptions& options = {}) { return Item(label.c_str(), options); }
    void Separator();

private:
    void PushStyle();
    void PopStyle();

    bool* m_openRef = nullptr;    // nullptr in OnLastItem and popupId modes
    bool m_idPushed = false;
    ContextMenuStyle m_style;
    int m_itemCounter = 0;
};
