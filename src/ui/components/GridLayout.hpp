#pragma once

#include <imgui.h>
#include <algorithm>

// ============================================================================
// Фиксированная вьюпортная матрица NxM на 100% экрана без скролла (GridLayout)
// ============================================================================
class GridLayout {
public:
    // RAII-скоуп для ячейки: if (auto cell = grid.Cell(col, row)) { ... }
    class CellScope {
    public:
        CellScope(GridLayout* parent, ImVec2 pos, ImVec2 size, int col, int row, bool active)
            : m_parent(parent), m_pos(pos), m_size(size), m_col(col), m_row(row), m_active(active) {}

        ~CellScope();

        // Запрет копирования
        CellScope(const CellScope&) = delete;
        CellScope& operator=(const CellScope&) = delete;

        // Перемещение
        CellScope(CellScope&& other) noexcept
            : m_parent(other.m_parent), m_pos(other.m_pos), m_size(other.m_size),
              m_col(other.m_col), m_row(other.m_row), m_active(other.m_active) {
            other.m_parent = nullptr;
            other.m_active = false;
        }

        explicit operator bool() const { return m_active; }
        float Width() const { return m_size.x; }
        float Height() const { return m_size.y; }
        ImVec2 Size() const { return m_size; }
        ImVec2 Pos() const { return m_pos; }
        int Col() const { return m_col; }
        int Row() const { return m_row; }

    private:
        GridLayout* m_parent = nullptr;
        ImVec2 m_pos{0, 0};
        ImVec2 m_size{0, 0};
        int m_col = 0;
        int m_row = 0;
        bool m_active = false;
    };

    // Конструктор: cols x rows ячеек. gapX и gapY по умолчанию берутся из UiTheme::Get().SpacingLarge()
    // totalSize <= 0 означает занять всю доступную область GetContentRegionAvail()
    GridLayout(int cols, int rows, float gapX = -1.0f, float gapY = -1.0f, ImVec2 totalSize = ImVec2(0, 0));
    ~GridLayout();

    // Запрет копирования
    GridLayout(const GridLayout&) = delete;
    GridLayout& operator=(const GridLayout&) = delete;

    // Перемещение (Move)
    GridLayout(GridLayout&& other) noexcept;
    GridLayout& operator=(GridLayout&& other) noexcept;

    // 1. Координатный доступ RAII (с поддержкой объединения colSpan / rowSpan):
    // if (auto cell = grid.Cell(0, 0, 1, 2)) { ... }
    CellScope Cell(int col, int row, int colSpan = 1, int rowSpan = 1);

    // 2. Потоковый доступ RAII (обход ячеек по порядку слева направо, сверху вниз):
    // while (auto cell = grid.NextCell()) { ... }
    CellScope NextCell();

    // 3. Классический Begin / End
    bool BeginCell(int col, int row, int colSpan = 1, int rowSpan = 1);
    void EndCell();

    // Геометрия
    int GetCols() const { return m_cols; }
    int GetRows() const { return m_rows; }
    float GetCellWidth() const { return m_cellWidth; }
    float GetCellHeight() const { return m_cellHeight; }
    float GetGapX() const { return m_gapX; }
    float GetGapY() const { return m_gapY; }
    ImVec2 GetTotalSize() const { return m_totalSize; }

    // Активные размеры ячейки в текущем потоке (автоматически опрашиваются Card)
    static float CurrentCellWidth();
    static float CurrentCellHeight();

private:
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

    void CalculateGeometry(ImVec2 userSize);

    static thread_local float s_currentCellWidth;
    static thread_local float s_currentCellHeight;
};
