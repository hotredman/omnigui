#pragma once

#include <imgui.h>
#include <string>

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"

namespace OmniKit {

// Navigation item options (designated initializers):
//
//     sidebar.Item({.label = "Live Stream", .icon = Icon::Gamepad, .selected = true});
//
// Identity is key, otherwise label.
struct SidebarItemOptions {
    std::string label;
    Icon        icon     = Icon::None;
    bool        selected = false;
    const char* key      = nullptr;
};

// Options for two-line list entry (title + date/status + LED):
//
//     list.Entry({.label = run.title, .sublabel = "10:14 • 1240 pts", .statusColor = run.ledColor, .key = run.id.c_str()});
struct SidebarEntryOptions {
    std::string label;
    std::string sublabel;
    ImU32       statusColor = 0;       // 0 — no LED indicator
    bool        selected    = false;
    const char* key         = nullptr;
    int         level       = 0;           // Tree nesting level (0 — no indent)
};

// Indentation for one tree level (base px)
constexpr float kSidebarTreeIndent = 14.0f;

// Response to clicking a tree node: expander arrow — Toggle, rest of row — Activate
enum class SidebarTreeClick { None, Toggle, Activate };

// Tree node options: expander arrow, title (up to two lines), right counter and
// status indicator. Node height expands with the number of title lines.
//
//     auto click = list.TreeNode({.label = "Project Alpha", .count = "12", .level = 0, .expanded = open});
//     if (click == SidebarTreeClick::Toggle) open = !open;
struct SidebarTreeNodeOptions {
    std::string label;
    std::string count;                     // Counter on the right; empty if none
    int         level       = 0;
    bool        expanded    = false;
    bool        selected    = false;
    ImU32       statusColor = 0;           // 0 — no indicator
    const char* key         = nullptr;
};

// Two-line title: first line broken at word boundary (words longer than line cut by char),
// second line holds the rest, ending with ellipsis if it overflows. Uses current ImGui font.
struct SidebarWrappedLabel {
    std::string line1;
    std::string line2;   // Empty if title fits on a single line
};
SidebarWrappedLabel SidebarWrapLabel(const char* text, float maxWidth);

// Scrollable list inside the sidebar (RAII scope). Created
// via Sidebar::ScrollList() and occupies all available space to the bottom of the panel.
class SidebarList : public Scope {
public:
    ~SidebarList();

    // Two-line list entry; returns true if clicked
    bool Entry(const SidebarEntryOptions& options);

    // Tree node with expander arrow and counter
    SidebarTreeClick TreeNode(const SidebarTreeNodeOptions& options);

    // Empty list placeholder
    void Empty(const std::string& message = "List is empty", const std::string& detail = "");

private:
    friend class Sidebar;
    SidebarList();
};

// ============================================================================
// Sidebar navigation panel (Sidebar). RAII scope: constructor opens
// the window, destructor closes it.
//
//     if (auto sidebar = Sidebar()) {
//         sidebar.Item({.label = "Live Stream", .icon = Icon::Gamepad}, Screen::Live, current);
//         sidebar.Separator();
//         sidebar.SectionTitle("RECORDED RUNS");
//         if (auto list = sidebar.ScrollList()) {
//             if (list.Entry({.label = "Run #01", .key = "run_01"})) { ... }
//         }
//     }
// ============================================================================
// Sidebar geometry (final px; default uses theme):
//
//     Sidebar({.posYPx = 80, .heightPx = 600});
struct SidebarOptions {
    float posYPx   = -1.0f;   // < 0 places below Header and Toolbar
    float heightPx = -1.0f;   // < 0 extends down to status bar
};

class Sidebar : public Scope {
public:
    // Geometry is computed from UiTheme::Get() by default
    explicit Sidebar(const SidebarOptions& options = {});
    ~Sidebar();

    // 1. Single-line menu item with vector icon; returns true if clicked
    bool Item(const SidebarItemOptions& options);

    // 2. Bind item to enum value: selected is computed automatically; on click,
    // current is set to value
    template<typename T>
    bool Item(SidebarItemOptions options, T value, T& current) {
        options.selected = (current == value);
        if (Item(options)) {
            current = value;
            return true;
        }
        return false;
    }

    // 3. Separator, section title, vertical spacer (base px)
    void Separator();
    void SectionTitle(const std::string& title);
    void Spacer(float basePx = 8.0f);

    // 4. Scrollable list below items (to the bottom of the panel)
    SidebarList ScrollList();

private:
    float Scale(float val) const { return UiTheme::Get().Scale(val); }
};

} // namespace OmniKit
