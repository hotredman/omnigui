#include "ui/components/FlowLayout.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

thread_local float FlowLayout::s_currentColumnWidth = 0.0f;

FlowLayout::ColumnScope::~ColumnScope() {
    if (m_parent && m_active) {
        m_parent->EndCol();
    }
}

void FlowLayout::ColumnScope::AlignWithInput() {
    const UiTheme& theme = UiTheme::Get();
    float labelH = ImGui::GetTextLineHeight() + theme.Scale(theme.input.labelSpacing);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + labelH);
}

void FlowLayout::ColumnScope::RightAlign(float itemWidth) {
    if (m_width > itemWidth) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (m_width - itemWidth));
    }
}

FlowLayout::FlowLayout(float availableWidth, float columnSpacing, float rowSpacing) {
    const UiTheme& theme = UiTheme::Get();
    m_columnSpacing = (columnSpacing >= 0.0f) ? columnSpacing : theme.SpacingLarge();
    m_rowSpacing = (rowSpacing >= 0.0f) ? rowSpacing : theme.CardRowSpacing();

    if (availableWidth > 0.0f) {
        m_totalAvailWidth = availableWidth;
    } else {
        float avail = ImGui::GetContentRegionAvail().x;
        m_totalAvailWidth = (avail > 0.0f) ? avail : 100.0f;
    }
    m_rowAvailWidth = m_totalAvailWidth;
}

FlowLayout::~FlowLayout() {
    if (m_colOpen) {
        EndCol();
    }
}

FlowLayout::FlowLayout(FlowLayout&& other) noexcept
    : m_totalAvailWidth(other.m_totalAvailWidth)
    , m_rowAvailWidth(other.m_rowAvailWidth)
    , m_columnSpacing(other.m_columnSpacing)
    , m_rowSpacing(other.m_rowSpacing)
    , m_rowSpanUsed(other.m_rowSpanUsed)
    , m_colIndexInRow(other.m_colIndexInRow)
    , m_accumulatedWidth(other.m_accumulatedWidth)
    , m_activeSpan(other.m_activeSpan)
    , m_activeWidth(other.m_activeWidth)
    , m_colOpen(other.m_colOpen)
    , m_needsNewRowSpacing(other.m_needsNewRowSpacing)
    , m_presetSpans(std::move(other.m_presetSpans))
{
    other.m_colOpen = false;
}

FlowLayout& FlowLayout::operator=(FlowLayout&& other) noexcept {
    if (this != &other) {
        if (m_colOpen) {
            EndCol();
        }
        m_totalAvailWidth = other.m_totalAvailWidth;
        m_rowAvailWidth = other.m_rowAvailWidth;
        m_columnSpacing = other.m_columnSpacing;
        m_rowSpacing = other.m_rowSpacing;
        m_rowSpanUsed = other.m_rowSpanUsed;
        m_colIndexInRow = other.m_colIndexInRow;
        m_accumulatedWidth = other.m_accumulatedWidth;
        m_activeSpan = other.m_activeSpan;
        m_activeWidth = other.m_activeWidth;
        m_colOpen = other.m_colOpen;
        m_needsNewRowSpacing = other.m_needsNewRowSpacing;
        m_presetSpans = std::move(other.m_presetSpans);
        other.m_colOpen = false;
    }
    return *this;
}

float FlowLayout::CalculateSpanWidth(int span) const {
    span = std::clamp(span, 1, 12);
    if (span == 12) {
        return m_rowAvailWidth;
    }
    float s = m_columnSpacing;
    float width = (span * m_rowAvailWidth - (12 - span) * s) / 12.0f;
    return std::max(20.0f, static_cast<float>(std::floor(width)));
}

float FlowLayout::CurrentColumnWidth() {
    return s_currentColumnWidth;
}

FlowLayout::ColumnScope FlowLayout::Col(::Col col) {
    bool ok = BeginCol(col);
    return ColumnScope(this, s_currentColumnWidth, ok);
}

bool FlowLayout::BeginCol(::Col col) {
    int span = std::clamp(col.span, 1, 12);

    if (m_colOpen) {
        EndCol();
    }

    // Автоперенос на новую строку: если на текущей строке уже есть колонки и новая не помещается в 12 долей
    if (m_rowSpanUsed > 0 && (m_rowSpanUsed + span > 12)) {
        m_rowSpanUsed = 0;
        m_accumulatedWidth = 0.0f;
        m_colIndexInRow = 0;
        m_needsNewRowSpacing = true;
    }

    // Переход на новую строку вниз с учетом вертикального отступа
    if (m_needsNewRowSpacing) {
        float avail = ImGui::GetContentRegionAvail().x;
        if (avail > 0.0f) {
            m_rowAvailWidth = avail;
        }
        float itemSpacingY = ImGui::GetStyle().ItemSpacing.y;
        if (m_rowSpacing != itemSpacingY && m_rowSpacing >= 0.0f) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (m_rowSpacing - itemSpacingY));
        }
        m_needsNewRowSpacing = false;
    }

    // Позиционирование по горизонтали: первая колонка идет на текущей строке, последующие через SameLine
    if (m_colIndexInRow > 0) {
        ImGui::SameLine(0.0f, m_columnSpacing);
    }

    // Расчет ширины: если колонка закрывает строку до 12 долей, отдаем точный остаток
    float width = 0.0f;
    if (m_rowSpanUsed + span >= 12) {
        float remaining = m_rowAvailWidth - m_accumulatedWidth - (m_colIndexInRow * m_columnSpacing);
        width = std::max(20.0f, remaining);
    } else {
        width = CalculateSpanWidth(span);
    }

    m_activeSpan = span;
    m_activeWidth = width;
    m_colOpen = true;
    s_currentColumnWidth = width;

    ImGui::SetNextItemWidth(width);
    ImGui::BeginGroup();
    return true;
}

void FlowLayout::EndCol() {
    if (!m_colOpen) {
        return;
    }

    ImGui::EndGroup();
    m_colOpen = false;
    s_currentColumnWidth = 0.0f;

    m_rowSpanUsed += m_activeSpan;
    m_accumulatedWidth += m_activeWidth;
    m_colIndexInRow++;

    // Если строка полностью заполнена 12 долями — следующая колонка начнется с новой строки
    if (m_rowSpanUsed >= 12) {
        m_rowSpanUsed = 0;
        m_accumulatedWidth = 0.0f;
        m_colIndexInRow = 0;
        m_needsNewRowSpacing = true;
    }
}

void FlowLayout::VerticalGap() {
    ImGui::Spacing();
}

void FlowLayout::NextRow() {
    if (m_colOpen) {
        EndCol();
    }
    m_rowSpanUsed = 0;
    m_accumulatedWidth = 0.0f;
    m_colIndexInRow = 0;
    m_needsNewRowSpacing = true;
}

FlowLayout FlowLayout::Columns2(float spacing) {
    FlowLayout layout(0.0f, spacing);
    layout.m_presetSpans = { 6, 6 };
    return layout;
}

FlowLayout FlowLayout::Columns3(float spacing) {
    FlowLayout layout(0.0f, spacing);
    layout.m_presetSpans = { 4, 4, 4 };
    return layout;
}

FlowLayout FlowLayout::Columns4(float spacing) {
    FlowLayout layout(0.0f, spacing);
    layout.m_presetSpans = { 3, 3, 3, 3 };
    return layout;
}

FlowLayout FlowLayout::Split(int spanLeft, int spanRight, float spacing) {
    FlowLayout layout(0.0f, spacing);
    layout.m_presetSpans = { spanLeft, spanRight };
    return layout;
}

bool FlowLayout::BeginColumn(int columnIndex) {
    if (columnIndex < 0 || columnIndex >= static_cast<int>(m_presetSpans.size())) {
        return false;
    }
    return BeginCol(::Col(m_presetSpans[columnIndex]));
}
