#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include <imgui.h>
#include <optional>
#include <string>
#include <vector>

namespace OmniKit {

// Card item row options
struct ItemRowOptions {
    const char* key     = "item";   // Row identity (ID and height cache)
    int         actions = 1;        // Number of action buttons on the right (reserves slot)
};

// Row title options (name is passed positionally)
struct ItemRowTitleOptions {
    const char* symbol = nullptr;   // Symbol in parentheses
    const char* unit   = nullptr;   // Units in brackets
};

// Badge options on the title line (text is passed positionally)
struct ItemRowBadgeOptions {
    UiVariant variant = UiVariant::Default;
};

// Options for action button on the right
struct ItemRowActionOptions {
    Icon        icon     = Icon::None;
    const char* tooltip  = nullptr;
    UiVariant   variant  = UiVariant::Secondary;
    bool        disabled = false;
};

// Composite interactive card-style item row in the evo-machine-cs style.
// Renders a list card with an individual frame, rounding, background, hover highlight,
// title with metadata (symbol, units), badges, right-aligned action buttons,
// word-wrapped description text, formulas, and interactive chip tags.
// RAII scope: constructor opens the card, destructor draws the background and closes it.
//
//     if (ItemRow row({.key = "density", .actions = 2}); row) {
//         row.Title("Density", {.symbol = "rho", .unit = "kg/m3"});
//         row.Badge("In method", {.variant = UiVariant::Success});
//         if (row.Action({.icon = Icon::Edit, .tooltip = "Edit"})) { ... }
//         row.Description("Mass per unit volume");
//     }
class ItemRow : public Scope {
public:
    explicit ItemRow(const ItemRowOptions& options = {});
    ~ItemRow();

    // Row title: name, symbol in parentheses, units in brackets
    void Title(const std::string& name, const ItemRowTitleOptions& options = {});

    // Semantic badge on the title line (automatically placed on the same line)
    void Badge(const char* text, const ItemRowBadgeOptions& options = {});

    // Action button (ToolButton) on the right; slots are reserved via ItemRowOptions::actions
    bool Action(const ItemRowActionOptions& options);

    // Item description (dimmed text wrapping to card width accounting for right action buttons)
    void Description(const std::string& text);

    // Formula expression ("symbol = expression")
    void Formula(const std::string& symbol, const std::string& expression);

    // Tags in evo-machine-cs style (# strength # tension ...);
    // returns the text of the tag clicked in this frame
    std::optional<std::string> Tags(const std::vector<std::string>& tags);

private:
    void DrawBadges();
    void Finish();

    struct BadgeItem {
        std::string text;
        UiVariant variant;
    };

    std::string m_id;
    int m_actionCount = 0;
    int m_totalActionsExpected = 1;
    ImVec2 m_startPos = ImVec2(0.0f, 0.0f);
    float m_availW = 0.0f;
    float m_padX = 0.0f;
    float m_padY = 0.0f;
    float m_prevHeight = 0.0f;

    float m_titleRowTopY = 0.0f;
    float m_titleRowHeight = 0.0f;
    float m_titleEndX = 0.0f;
    std::vector<BadgeItem> m_badges;

    ImDrawListSplitter m_splitter;
};

} // namespace OmniKit
