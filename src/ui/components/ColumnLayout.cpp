#include "ui/components/ColumnLayout.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

thread_local float ColumnLayout::s_currentColumnWidth = 0.0f;

ColumnLayout::ColumnScope::~ColumnScope() {
    if (m_parent && m_open) {
        m_parent->EndCol();
    }
}

void ColumnLayout::ColumnScope::AlignWithInput() {
    const UiTheme& theme = UiTheme::Get();
    float labelH = ImGui::GetTextLineHeight() + theme.Scale(theme.input.labelSpacing);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + labelH);
}

void ColumnLayout::ColumnScope::RightAlign(float itemWidth) {
    if (m_width > itemWidth) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (m_width - itemWidth));
    }
}

ColumnLayout::ColumnLayout(const ColumnLayoutOptions& options) {
    m_open = true;
    const UiTheme& theme = UiTheme::Get();
    m_columnSpacing = (options.columnGap >= 0.0f) ? theme.Scale(options.columnGap) : theme.SpacingLarge();
    m_rowSpacing = (options.rowGap >= 0.0f) ? theme.Scale(options.rowGap) : theme.CardRowSpacing();

    if (options.widthPx > 0.0f) {
        m_totalAvailWidth = options.widthPx;
    } else {
        float avail = ImGui::GetContentRegionAvail().x;
        m_totalAvailWidth = (avail > 0.0f) ? avail : 100.0f;
    }
    m_rowAvailWidth = m_totalAvailWidth;
}

ColumnLayout::~ColumnLayout() {
    if (m_colOpen) {
        EndCol();
    }
}

float ColumnLayout::CalculateSpanWidth(int span) const {
    return SpanWidth(span, m_rowAvailWidth, m_columnSpacing);
}

float ColumnLayout::CurrentColumnWidth() {
    return s_currentColumnWidth;
}

ColumnLayout::ColumnScope ColumnLayout::Col(::Col col) {
    bool ok = BeginCol(col);
    return ColumnScope(this, s_currentColumnWidth, ok);
}

bool ColumnLayout::BeginCol(::Col col) {
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

void ColumnLayout::EndCol() {
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

void ColumnLayout::NextRow() {
    if (m_colOpen) {
        EndCol();
    }
    m_rowSpanUsed = 0;
    m_accumulatedWidth = 0.0f;
    m_colIndexInRow = 0;
    m_needsNewRowSpacing = true;
}