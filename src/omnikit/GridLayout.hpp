#pragma once

#include "omnikit/Scope.hpp"
#include <imgui.h>
#include <algorithm>
#include <optional>

namespace OmniKit {

// Grid layout options (designated initializers):
//
//     GridLayout grid({.cols = 2, .rows = 2});
//     if (auto cell = grid.Cell(0, 0, 2, 1)) { chart.Render(cell.Size()); }   // 2-column wide cell
struct GridLayoutOptions {
    int                   cols  = 1;
    int                   rows  = 1;
    float                 gapX  = -1.0f;   // base px; < 0 — UiTheme::SpacingLarge()
    float                 gapY  = -1.0f;
    std::optional<ImVec2> sizePx;          // resolved total grid size, px; defaults to fill available space
};

// ============================================================================
// Rectangular NxM grid filling available space (GridLayout, RAII scope)
// ============================================================================
class GridLayout : public Scope {
public:
    // RAII cell for loops: if (auto cell = grid.Cell(col, row)) { ... }
    class CellScope : public Scope {
    public:
        ~CellScope();

        float Width() const { return m_size.x; }
        float Height() const { return m_size.y; }
        ImVec2 Size() const { return m_size; }
        ImVec2 Pos() const { return m_pos; }
        int Col() const { return m_col; }
        int Row() const { return m_row; }

    private:
        friend class GridLayout;
        CellScope(GridLayout* parent, ImVec2 pos, ImVec2 size, int col, int row, bool active)
            : m_parent(parent), m_pos(pos), m_size(size), m_col(col), m_row(row) {
            m_open = active;
        }

        GridLayout* m_parent = nullptr;
        ImVec2 m_pos{0, 0};
        ImVec2 m_size{0, 0};
        int m_col = 0;
        int m_row = 0;
    };

    explicit GridLayout(const GridLayoutOptions& options = {});
    ~GridLayout();

    // 1. Cell by coordinates, with optional colSpan / rowSpan:
    // if (auto cell = grid.Cell(0, 0, 1, 2)) { ... }
    CellScope Cell(int col, int row, int colSpan = 1, int rowSpan = 1);

    // 2. Next cell in sequence (left-to-right, top-to-bottom):
    // while (auto cell = grid.NextCell()) { ... }
    CellScope NextCell();

    // Geometry
    int GetCols() const { return m_cols; }
    int GetRows() const { return m_rows; }
    float GetCellWidth() const { return m_cellWidth; }
    float GetCellHeight() const { return m_cellHeight; }
    float GetGapX() const { return m_gapX; }
    float GetGapY() const { return m_gapY; }
    ImVec2 GetTotalSize() const { return m_totalSize; }

    // Active cell dimensions in current thread (queried by components, e.g. Card)
    static float CurrentCellWidth();
    static float CurrentCellHeight();

private:
    bool BeginCell(int col, int row, int colSpan, int rowSpan);
    void EndCell();
    void CalculateGeometry(ImVec2 userSize);

    int m_cols = 1;
    int m_rows = 1;
    float m_gapX = 16.0f;
    float m_gapY = 16.0f;
    ImVec2 m_originScreenPos{0, 0};
    ImVec2 m_totalSize{0, 0};
    float m_cellWidth = 0.0f;
    float m_cellHeight = 0.0f;

    int m_nextCol = 0;
    int m_nextRow = 0;
    bool m_cellOpen = false;

    static thread_local float s_currentCellWidth;
    static thread_local float s_currentCellHeight;
};

} // namespace OmniKit
