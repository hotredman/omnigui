#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include <imgui.h>

namespace OmniKit {

// Filter bar options
struct FilterBarOptions {
    const char* key    = "##FilterBar";   // bar identity
    float       height = 0.0f;            // base px; 0 — theme.filterBar.height
};

// Filter bar label options (text passed positionally)
struct FilterBarLabelOptions {
    UiVariant variant = UiVariant::Default;
};

// In-page filter, search, and action bar (FilterBar).
// Renders inside the content work area (unlike the global shell Toolbar).
// RAII scope; Left and Right zones are nested RAII scopes:
//
//     if (FilterBar bar({.key = "filters"}); bar) {
//         if (auto left = bar.Left()) { FilterBar::Label("Search:"); ... }
//         if (auto right = bar.Right()) { ... }
//     }
class FilterBar : public Scope {
public:
    // Left bar zone (scope opens in constructor and closes in destructor)
    class LeftScope : public Scope {
    public:
        explicit LeftScope(FilterBar& parent);
        ~LeftScope();
    private:
        FilterBar* m_parent;
    };

    // Right bar zone, aligned to right edge
    class RightScope : public Scope {
    public:
        explicit RightScope(FilterBar& parent);
        ~RightScope();
    private:
        FilterBar* m_parent;
    };

    explicit FilterBar(const FilterBarOptions& options = {});
    ~FilterBar();

    LeftScope Left() { return LeftScope(*this); }
    RightScope Right() { return RightScope(*this); }

    float GetAvailableWidth() const { return m_availWidth; }

    // Available width for a stretching element in the left group (in base unscaled pixels).
    // If minWidth <= 0, theme.filterBar.minSearchWidth is used
    float GetStretchWidth(float minWidth = 0.0f) const;

    // Spacing between items in the same group (theme.filterBar.itemSpacing)
    static void NextItem();

    // Spacing between different semantic groups (theme.filterBar.groupSpacing)
    static void NextGroup();

    // Arbitrary spacing between items (base px)
    static void Spacing(float spacing);

    // Text label inside the bar (vertically centered relative to UiSize::Small controls).
    // Automatically advances cursor to the associated control with theme.filterBar.labelSpacing gap
    static void Label(const char* text, const FilterBarLabelOptions& options = {});

private:
    friend class LeftScope;
    friend class RightScope;

    const char* m_id;
    float m_baseHeight;
    float m_actualHeight = 0.0f;

    ImVec2 m_startPos = ImVec2(0, 0);
    float m_availWidth = 0.0f;
    float m_leftWidth = 0.0f;
    float m_rightWidth = 0.0f;
    ImGuiID m_storageId = 0;
};

} // namespace OmniKit
