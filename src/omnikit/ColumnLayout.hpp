#pragma once

#include "omnikit/Scope.hpp"
#include <imgui.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace OmniKit {

// ============================================================================
// Row span in a 12-column grid (Bootstrap / Grid style)
// ============================================================================
struct Col {
    int span = 6; // Defaults to 6 out of 12 (two-column mode = 50% width)
    float reservedPx = 0.0f; // For Col::Fill(): reserved width on the right (e.g., for buttons)

    constexpr Col() : span(6), reservedPx(0.0f) {}
    constexpr Col(int s) : span(s < -1 ? -1 : (s > 12 ? 12 : s)), reservedPx(0.0f) {}
    constexpr Col(int s, float reserved) : span(s < -1 ? -1 : (s > 12 ? 12 : s)), reservedPx(reserved) {}

    static constexpr Col Auto()                         { return Col{0, 0.0f};  } // Automatic content-based width (unconstrained by grid)
    static constexpr Col Fit()                          { return Col{0, 0.0f};  } // Synonym for Auto
    static constexpr Col Fill(float reservedRight = 0.0f){ return Col{-1, reservedRight}; } // Fills remaining row width (with optional right margin reserved for buttons)
    static constexpr Col Full()                         { return Col{12, 0.0f}; } // 100% of row
    static constexpr Col Half()                         { return Col{6, 0.0f};  } // 50% of row (2 columns)
    static constexpr Col Third()                        { return Col{4, 0.0f};  } // 33.3% of row (3 columns)
    static constexpr Col TwoThirds()                    { return Col{8, 0.0f};  } // 66.7% of row (2/3)
    static constexpr Col Quarter()                      { return Col{3, 0.0f};  } // 25% of row (4 columns)
    static constexpr Col Sixth()                        { return Col{2, 0.0f};  } // 16.7% of row (6 columns)
    static constexpr Col Custom(int s)                  { return Col{s, 0.0f};  }

    bool IsAuto() const { return span == 0; }
    bool IsFill() const { return span < 0; }
};

// Column width for span units out of 12 given available width avail and spacing gap (all values in px).
// Shared formula for ColumnLayout and Card.
inline float SpanWidth(int span, float avail, float gap) {
    span = std::clamp(span, 1, 12);
    if (span == 12) {
        return avail;
    }
    const float width = (span * avail - (12 - span) * gap) / 12.0f;
    return std::max(20.0f, std::floor(width));
}

// ============================================================================
// Responsive 12-column layout with auto-wrapping rows (ColumnLayout).
// RAII scope: columns are created via Col(), rows wrap automatically.
//
//     ColumnLayout row({.columnGap = 12});
//     if (auto c = row.Col(Col::TwoThirds())) { ... }
//     if (auto c = row.Col(Col::Third()))     { ... }   // row is full; subsequent items start a new row
// ============================================================================
struct ColumnLayoutOptions {
    float widthPx   = 0.0f;    // resolved px; 0 — full available width
    float columnGap = -1.0f;   // base px; < 0 — UiTheme::SpacingLarge()
    float rowGap    = -1.0f;   // base px; < 0 — UiTheme::CardRowSpacing()
};

class ColumnLayout : public Scope {
public:
    // RAII column: if (auto c = row.Col(Col::Half())) { ... }
    class ColumnScope : public Scope {
    public:
        ~ColumnScope();

        float Width() const { return m_width; }
        void AlignWithInput();
        void RightAlign(float itemWidth);

    private:
        friend class ColumnLayout;
        ColumnScope(ColumnLayout* parent, float width, bool active)
            : m_parent(parent), m_width(width) {
            m_open = active;
        }

        ColumnLayout* m_parent = nullptr;
        float m_width = 0.0f;
    };

    explicit ColumnLayout(const ColumnLayoutOptions& options = {});
    ~ColumnLayout();

    ColumnScope Col(OmniKit::Col col = OmniKit::Col::Half());

    // Explicitly advances to a new row
    void NextRow();

    // Geometry
    float GetAvailableWidth() const { return m_rowAvailWidth; }
    float CalculateSpanWidth(int span) const;
    float GetColumnSpacing() const { return m_columnSpacing; }
    float GetRowSpacing() const { return m_rowSpacing; }
    int GetRemainingSpan() const { return 12 - m_rowSpanUsed; }

    // Active column width in current thread (queried by components, e.g. Card)
    static float CurrentColumnWidth();

private:
    bool BeginCol(OmniKit::Col col);
    void EndCol();

    float m_totalAvailWidth = 0.0f;
    float m_rowAvailWidth = 0.0f;
    float m_columnSpacing = 16.0f;
    float m_rowSpacing = 16.0f;

    int m_rowSpanUsed = 0;
    int m_colIndexInRow = 0;
    float m_accumulatedWidth = 0.0f;
    int m_activeSpan = 0;
    float m_activeWidth = 0.0f;
    bool m_colOpen = false;
    bool m_needsNewRowSpacing = false;

    static thread_local float s_currentColumnWidth;
};
} // namespace OmniKit
