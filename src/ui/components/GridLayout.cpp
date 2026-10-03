#include "ui/components/GridLayout.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

thread_local float GridLayout::s_currentCellWidth = 0.0f;
thread_local float GridLayout::s_currentCellHeight = 0.0f;

GridLayout::CellScope::~CellScope() {
    if (m_parent && m_open) {
        m_parent->EndCell();
    }
}

GridLayout::GridLayout(const GridLayoutOptions& options)
    : m_cols(std::max(1, options.cols))
    , m_rows(std::max(1, options.rows))
{
    m_open = true;
    const UiTheme& theme = UiTheme::Get();
    m_gapX = (options.gapX >= 0.0f) ? theme.Scale(options.gapX) : theme.SpacingLarge();
    m_gapY = (options.gapY >= 0.0f) ? theme.Scale(options.gapY) : theme.SpacingLarge();

    CalculateGeometry(options.sizePx ? *options.sizePx : ImVec2(0.0f, 0.0f));
}

GridLayout::~GridLayout() {
    if (m_cellOpen) {
        EndCell();
    }
    // Восстанавливаем позицию курсора под всей областью сетки и регистрируем габарит в ImGui
    ImGui::SetCursorScreenPos(ImVec2(m_originScreenPos.x, m_originScreenPos.y + m_totalSize.y));
    ImGui::Dummy(ImVec2(m_totalSize.x, 0.0f));
}

void GridLayout::CalculateGeometry(ImVec2 userSize) {
    m_originScreenPos = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    m_totalSize.x = (userSize.x > 0.0f) ? userSize.x : std::max(50.0f, avail.x);
    m_totalSize.y = (userSize.y > 0.0f) ? userSize.y : std::max(50.0f, avail.y);

    float netW = std::max(20.0f, m_totalSize.x - (m_cols - 1) * m_gapX);
    float netH = std::max(20.0f, m_totalSize.y - (m_rows - 1) * m_gapY);

    m_cellWidth = std::floor(netW / static_cast<float>(m_cols));
    m_cellHeight = std::floor(netH / static_cast<float>(m_rows));
}

float GridLayout::CurrentCellWidth() {
    return s_currentCellWidth;
}

float GridLayout::CurrentCellHeight() {
    return s_currentCellHeight;
}

GridLayout::CellScope GridLayout::Cell(int col, int row, int colSpan, int rowSpan) {
    bool ok = BeginCell(col, row, colSpan, rowSpan);
    ImVec2 pos(0, 0);
    ImVec2 size(s_currentCellWidth, s_currentCellHeight);
    if (ok) {
        pos = ImGui::GetCursorScreenPos();
    }
    return CellScope(this, pos, size, col, row, ok);
}

GridLayout::CellScope GridLayout::NextCell() {
    if (m_nextRow >= m_rows) {
        return CellScope(this, ImVec2(0, 0), ImVec2(0, 0), -1, -1, false);
    }

    int c = m_nextCol;
    int r = m_nextRow;

    m_nextCol++;
    if (m_nextCol >= m_cols) {
        m_nextCol = 0;
        m_nextRow++;
    }

    return Cell(c, r, 1, 1);
}

bool GridLayout::BeginCell(int col, int row, int colSpan, int rowSpan) {
    if (m_cellOpen) {
        EndCell();
    }

    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) {
        return false;
    }

    colSpan = std::clamp(colSpan, 1, m_cols - col);
    rowSpan = std::clamp(rowSpan, 1, m_rows - row);

    float posX = m_originScreenPos.x + col * (m_cellWidth + m_gapX);
    float posY = m_originScreenPos.y + row * (m_cellHeight + m_gapY);

    float width = 0.0f;
    float height = 0.0f;

    // Пиксель-в-пиксель: если ячейка закрывает правый или нижний край сетки, притягиваем точно к краю
    if (col + colSpan >= m_cols) {
        width = std::max(10.0f, m_totalSize.x - (posX - m_originScreenPos.x));
    } else {
        width = colSpan * m_cellWidth + (colSpan - 1) * m_gapX;
    }

    if (row + rowSpan >= m_rows) {
        height = std::max(10.0f, m_totalSize.y - (posY - m_originScreenPos.y));
    } else {
        height = rowSpan * m_cellHeight + (rowSpan - 1) * m_gapY;
    }

    ImGui::SetCursorScreenPos(ImVec2(posX, posY));

    m_cellOpen = true;
    s_currentCellWidth = width;
    s_currentCellHeight = height;

    ImGui::SetNextItemWidth(width);
    ImGui::BeginGroup();
    return true;
}

void GridLayout::EndCell() {
    if (!m_cellOpen) {
        return;
    }

    ImGui::EndGroup();
    m_cellOpen = false;
    s_currentCellWidth = 0.0f;
    s_currentCellHeight = 0.0f;
}
