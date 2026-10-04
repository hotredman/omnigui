#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include <imgui.h>
#include <string>
#include <functional>

namespace OmniKit {

// List options (designated initializers). Sizes are in base px, 0 fills available space:
//
//     if (List list({.key = "runs", .height = 240}); list) { ... }
struct ListOptions {
    const char*      key    = "##List";   // Container identity
    float            width  = 0.0f;
    float            height = 0.0f;
    const ListStyle* style  = nullptr;    // Style override; nullptr uses theme
};

// Section header options. Title is passed positionally:
//
//     if (list.Header("RUNS", {.actionIcon = "+", .badge = "3"})) { /* action button clicked */ }
struct ListHeaderOptions {
    const char* actionIcon = nullptr;   // Action button on the right, e.g. "+"
    const char* badge      = nullptr;   // Counter badge on the right, e.g. "3"
};

// List item options. Identity is key, otherwise label:
//
//     if (list.Item({.label = "Run #01", .sublabel = "10:14", .rightText = "1240 pts", .selected = true})) { ... }
struct ListItemOptions {
    const char* label     = nullptr;
    const char* sublabel  = nullptr;   // Second line; if present, item is taller
    const char* rightText = nullptr;   // Label on the right
    Icon        icon      = Icon::None;
    ImU32       iconColor = 0;         // Icon color; without icon, used as LED indicator dot
    bool        selected  = false;
    float       height    = 0.0f;      // Base px; 0 uses style default
    const char* key       = nullptr;
};

// Universal vertical list component (List / ListView).
// Supports section headers, single selection, icons, status indicators,
// right-aligned labels, and empty states. Fully encapsulates ImGui.
// RAII scope: constructor opens scrollable container, destructor closes it.
class List : public Scope {
public:
    explicit List(const ListOptions& options = {});
    ~List();

    // List section header with optional counter and action button on the right;
    // returns true if the action button was clicked in this frame
    bool Header(const char* title, const ListHeaderOptions& options = {});

    // List item (single or multi-line) supporting selection, icon/LED, and right-aligned text
    bool Item(const ListItemOptions& options);

    // Empty state display
    void Empty(const char* message = "List is empty", const char* detail = nullptr);

    // Thin horizontal separator
    void Separator();

private:
    const ListStyle* m_style = nullptr;
};

} // namespace OmniKit
