#include "ui/components/Card.hpp"
#include "ui/components/DisabledScope.hpp"
#include "ui/components/Button.hpp"
#include "ui/components/InputField.hpp"
#include "ui/components/Combo.hpp"
#include "ui/components/Toggle.hpp"
#include "ui/components/ValueDisplay.hpp"
#include "ui/components/ColumnLayout.hpp"
#include "ui/components/GridLayout.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cfloat>

// ============================================================================
// RAII Scopes
// ============================================================================
Card::RowScope::RowScope(Card* parent, std::optional<UiSize> size)
    : m_parent(parent)
{
    if (m_parent) {
        m_parent->BeginRow(size);
        m_open = true;
    }
}

Card::RowScope::~RowScope() {
    if (m_parent) {
        m_parent->EndRow();
    }
}

Card::ColumnScope::ColumnScope(Card* parent, ::Col col)
    : m_parent(parent)
    , m_col(col)
{
    if (m_parent) {
        m_parent->PrepareField(col, m_width);
        ImGui::BeginGroup();
        m_open = true;
    }
}

Card::ColumnScope::~ColumnScope() {
    if (m_parent) {
        ImGui::EndGroup();
        m_parent->FinishField(m_col);
    }
}

// ============================================================================
// Card Lifecycle
// ============================================================================

Card::Card(const CardOptions& options)
    : m_stretchY(options.stretchY)
    , m_style(options.style ? *options.style : UiTheme::Get().card)
{
    const UiTheme& theme = UiTheme::Get();
    const char* title = options.title;
    ImGuiID childId = 0;
    if (options.key) {
        if (options.key.type == UiKey::Type::String) childId = ImHashStr(options.key.str);
        else if (options.key.type == UiKey::Type::Int) childId = ImHashData(&options.key.index, sizeof(int));
        else if (options.key.type == UiKey::Type::Ptr) childId = ImHashData(&options.key.ptr, sizeof(void*));
    } else if (title && title[0] != '\0') {
        childId = ImHashStr(title);
    } else {
        childId = ImHashStr("##card");
    }
    ImVec2 size(theme.Scale(options.width), theme.Scale(options.height));

    if (size.x <= 0.0f) {
        float gridW = GridLayout::CurrentCellWidth();
        float flowW = ColumnLayout::CurrentColumnWidth();
        if (gridW > 0.0f) {
            size.x = gridW;
        } else if (flowW > 0.0f) {
            size.x = flowW;
        }
    }
    if (size.y <= 0.0f && !m_stretchY) {
        float gridH = GridLayout::CurrentCellHeight();
        if (gridH > 0.0f) {
            size.y = gridH;
        }
    }

    if (m_stretchY) {
        float availH = ImGui::GetContentRegionAvail().y;
        size.y = std::max(100.0f, availH);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, theme.Scale(m_style.cornerRadius));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, m_style.borderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(m_style.paddingX), theme.Scale(m_style.paddingY)));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(m_style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(m_style.colBorder).Value);

    ImGuiChildFlags childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
    if (!m_stretchY && size.y <= 0.0f) {
        childFlags |= ImGuiChildFlags_AutoResizeY;
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    m_open = ImGui::BeginChild(childId, size, childFlags, windowFlags);
    // Фон, рамка, скругление и отступы применены к окну карточки; дальше —
    // содержимое, которое их не наследует (вложенный список не перекрашивает фон)
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
    m_ended = false;
    m_rowSpanUsed = 0;
    m_columnSpacing = theme.Scale(m_style.columnSpacing);
    m_rowSpacing = theme.Scale(m_style.rowSpacing);
    m_rowMaxY = 0.0f;
    m_currentRowHeight = 0.0f;
    m_needsRowAdvance = false;

    if (title && title[0] != '\0') {
        ImFont* font = m_style.titleFont ? m_style.titleFont : theme.fontBold;
        theme.PushFont(font, m_style.titleFontSize);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 textSize = ImGui::CalcTextSize(title);

        dl->AddText(pos, m_style.colTitle, title);
        ImGui::Dummy(ImVec2(textSize.x, textSize.y));
        theme.PopFont();

        // Запоминаем границу заголовка для отложенного перехода перед первым полем
        m_rowMaxY = pos.y + textSize.y;
        m_rowSpacing = theme.Scale(m_style.titleSpacing);
        m_needsRowAdvance = true;
    }
}

Card::~Card() {
    End();
}

void Card::End() {
    if (!m_ended) {
        m_needsRowAdvance = false;
        ImGui::EndChild();
        m_ended = true;
    }
}

// ============================================================================
// Grid Geometry & Rows
// ============================================================================

float Card::GetFullWidth() const {
    float avail = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    return (avail > 0.0f) ? avail : 100.0f;
}

float Card::GetRemainingHeight() const {
    float avail = ImGui::GetContentRegionAvail().y;
    return std::max(40.0f, avail);
}

float Card::CalculateSpanWidth(int span) const {
    return SpanWidth(span, GetFullWidth(), m_columnSpacing);
}

Card::RowScope Card::Row(std::optional<UiSize> size) {
    return RowScope(this, size);
}

Card::ColumnScope Card::Col(::Col col) {
    return ColumnScope(this, col);
}

void Card::BeginRow(std::optional<UiSize> size) {
    if (m_rowSpanUsed > 0 || m_inAutoRow) {
        NextRow();
    }
    m_currentRowHeight = size ? UiTheme::Get().GetMetrics(*size).height : 0.0f;
}

void Card::EndRow() {
    if (m_rowSpanUsed > 0 || m_inAutoRow) {
        NextRow();
    }
    m_currentRowHeight = 0.0f;
}

float Card::ResolveHeight(std::optional<UiSize> size, std::optional<UiSize> fallback) const {
    const UiTheme& theme = UiTheme::Get();
    if (size) {
        return theme.GetMetrics(*size).height;
    }
    if (m_currentRowHeight > 0.0f) {
        return m_currentRowHeight;
    }
    if (fallback) {
        return theme.GetMetrics(*fallback).height;
    }
    return 0.0f;
}

void Card::PrepareField(::Col col, float& outWidth) {
    if (col.IsFill()) {
        // Если до этого были обычные элементы сетки — завершаем строку сетки
        if (!m_inAutoRow && m_rowSpanUsed > 0) {
            m_rowSpanUsed = 0;
            m_rowStartX = 0.0f;
            m_rowStartY = 0.0f;
            m_needsRowAdvance = true;
        }

        // Отложенный переход на следующую строку
        if (m_needsRowAdvance && m_rowMaxY > 0.0f) {
            float startScreenX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
            float nextRowScreenY = m_rowMaxY + m_rowSpacing;
            ImGui::SetCursorScreenPos(ImVec2(startScreenX, nextRowScreenY));
            m_rowSpacing = UiTheme::Get().Scale(m_style.rowSpacing);
            m_needsRowAdvance = false;
            m_rowMaxY = 0.0f;
            m_inAutoRow = false;
        }

        float rightScreenX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        float availOnLine = 0.0f;

        if (!m_inAutoRow) {
            availOnLine = GetFullWidth();
            m_rowStartX = ImGui::GetCursorPosX();
            m_rowStartY = ImGui::GetCursorPosY();
            m_rowMaxY = ImGui::GetCursorScreenPos().y;
            m_inAutoRow = true;
        } else {
            float prevRightScreenX = ImGui::GetItemRectMax().x;
            availOnLine = std::max(0.0f, rightScreenX - prevRightScreenX - m_columnSpacing);
            ImGui::SameLine(0.0f, m_columnSpacing);
            ImGui::SetCursorPosY(m_rowStartY);
        }

        float reserved = col.reservedPx;
        float reservedTotal = (reserved > 0.0f) ? (reserved + m_columnSpacing) : 0.0f;
        if (availOnLine > reservedTotal + 20.0f) {
            outWidth = availOnLine - reservedTotal;
        } else {
            outWidth = std::max(20.0f, availOnLine);
        }
        return;
    }

    if (col.IsAuto()) {
        // Если до этого были обычные элементы сетки — завершаем строку сетки
        if (!m_inAutoRow && m_rowSpanUsed > 0) {
            m_rowSpanUsed = 0;
            m_rowStartX = 0.0f;
            m_rowStartY = 0.0f;
            m_needsRowAdvance = true;
        }

        // Проверяем, помещается ли элемент в текущую строку auto-элементов
        if (m_inAutoRow) {
            float rightScreenX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
            float prevRightScreenX = ImGui::GetItemRectMax().x;
            float remaining = rightScreenX - prevRightScreenX - m_columnSpacing;
            if (outWidth > 0.0f && outWidth > remaining + 2.0f) {
                // Не помещается — переносим на следующую строку
                NextRow();
            }
        }

        // Отложенный переход на следующую строку
        if (m_needsRowAdvance && m_rowMaxY > 0.0f) {
            float startScreenX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
            float nextRowScreenY = m_rowMaxY + m_rowSpacing;
            ImGui::SetCursorScreenPos(ImVec2(startScreenX, nextRowScreenY));
            m_rowSpacing = UiTheme::Get().Scale(m_style.rowSpacing);
            m_needsRowAdvance = false;
            m_rowMaxY = 0.0f;
            m_inAutoRow = false;
        }

        if (!m_inAutoRow) {
            // Первый auto-элемент в строке
            m_rowStartX = ImGui::GetCursorPosX();
            m_rowStartY = ImGui::GetCursorPosY();
            m_rowMaxY = ImGui::GetCursorScreenPos().y;
            m_inAutoRow = true;
        } else {
            // Последующий auto-элемент в строке
            ImGui::SameLine(0.0f, m_columnSpacing);
            ImGui::SetCursorPosY(m_rowStartY);
        }
        return;
    }

    // Обработка обычных элементов сетки (Col 1..12):
    // Если до этого была строка auto-элементов — переходим на новую строку
    if (m_inAutoRow) {
        NextRow();
        m_inAutoRow = false;
    }

    int span = std::clamp(col.span, 1, 12);

    // Автоперенос (Wrap): если на текущей строке уже есть элементы и новый не помещается
    if (m_rowSpanUsed > 0 && (m_rowSpanUsed + span > 12)) {
        m_rowSpanUsed = 0;
        m_rowStartX = 0.0f;
        m_rowStartY = 0.0f;
        m_needsRowAdvance = true;
    }

    // Отложенный переход на следующую строку перед отрисовкой нового поля
    if (m_needsRowAdvance && m_rowMaxY > 0.0f) {
        float startScreenX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
        float nextRowScreenY = m_rowMaxY + m_rowSpacing;
        ImGui::SetCursorScreenPos(ImVec2(startScreenX, nextRowScreenY));
        m_rowSpacing = UiTheme::Get().Scale(m_style.rowSpacing);
        m_needsRowAdvance = false;
        m_rowMaxY = 0.0f;
    }

    float fullW = GetFullWidth();
    float s = m_columnSpacing;

    if (m_rowSpanUsed == 0) {
        // Начало строки: фиксируем базовый X и Y
        m_rowStartX = ImGui::GetCursorPosX();
        m_rowStartY = ImGui::GetCursorPosY();
        m_rowMaxY = ImGui::GetCursorScreenPos().y;
    } else {
        // Позиционируем строго СРАЗУ за левым элементом
        float leftWidth = CalculateSpanWidth(m_rowSpanUsed);
        float nextPosX = m_rowStartX + leftWidth + s;
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::SetCursorPosX(nextPosX);
        ImGui::SetCursorPosY(m_rowStartY);
    }

    // Если элемент завершает строку до 12 долей, отдаем ему ровно оставшуюся ширину до правого края карточки
    if (m_rowSpanUsed + span >= 12) {
        float currentOffset = (m_rowSpanUsed == 0) ? 0.0f : (CalculateSpanWidth(m_rowSpanUsed) + s);
        outWidth = std::max(20.0f, fullW - currentOffset);
    } else {
        outWidth = CalculateSpanWidth(span);
    }
}

void Card::FinishField(::Col col) {
    // Учитываем нижний край только что отрисованного поля
    float fieldBottom = ImGui::GetItemRectMax().y;
    if (fieldBottom > m_rowMaxY) {
        m_rowMaxY = fieldBottom;
    }

    if (col.IsAuto() || col.IsFill()) {
        m_inAutoRow = true;
        return;
    }

    int span = std::clamp(col.span, 1, 12);
    m_rowSpanUsed += span;

    // Если строка заполнена полностью (12 долей) — помечаем необходимость перехода на новую строку.
    // Курсор не перемещается здесь, чтобы не оставлять незаполненный SetCursorScreenPos перед EndChild()
    if (m_rowSpanUsed >= 12) {
        m_rowSpanUsed = 0;
        m_rowStartX = 0.0f;
        m_rowStartY = 0.0f;
        m_needsRowAdvance = true;
    }
}

void Card::NextRow() {
    m_rowSpanUsed = 0;
    m_rowStartX = 0.0f;
    m_rowStartY = 0.0f;
    m_inAutoRow = false;

    if (m_rowMaxY > 0.0f) {
        m_needsRowAdvance = true;
    }
}

// ============================================================================
// ContentList & Card Helpers
// ============================================================================

Card::ContentListScope::ContentListScope(Card* parent, const char* key)
    : m_parent(parent)
{
    if (m_parent) {
        m_open = m_parent->BeginContentList(key);
    }
}

// EndChild обязателен и тогда, когда BeginChild вернул false (требование ImGui)
Card::ContentListScope::~ContentListScope() {
    if (m_parent) {
        m_parent->EndContentList();
    }
}

bool Card::BeginContentList(const char* key) {
    float availH = GetRemainingHeight();
    return ImGui::BeginChild(key ? key : "##content_list", ImVec2(0.0f, availH), false);
}

void Card::EndContentList() {
    ImGui::EndChild();
}

void Card::Empty(const char* message, const char* detail) {
    if (!message) return;
    const UiTheme& theme = UiTheme::Get();

    ImGui::Dummy(ImVec2(1.0f, theme.Scale(20.0f)));

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImFont* font = theme.fontMedium ? theme.fontMedium : theme.defaultFont;
    theme.PushFont(font, 15.0f);

    ImVec2 msgSz = ImGui::CalcTextSize(message);
    float msgX = ImGui::GetCursorPosX() + (avail.x - msgSz.x) * 0.5f;
    if (msgX > ImGui::GetCursorPosX()) {
        ImGui::SetCursorPosX(msgX);
    }
    ImGui::PushStyleColor(ImGuiCol_Text, theme.palette.textMuted);
    ImGui::TextUnformatted(message);
    ImGui::PopStyleColor();
    theme.PopFont();

    if (detail && detail[0] != '\0') {
        ImFont* subFont = theme.fontRegular ? theme.fontRegular : theme.defaultFont;
        theme.PushFont(subFont, 13.0f);
        ImVec2 detSz = ImGui::CalcTextSize(detail);
        float detX = ImGui::GetCursorPosX() + (avail.x - detSz.x) * 0.5f;
        if (detX > ImGui::GetCursorPosX()) {
            ImGui::SetCursorPosX(detX);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, theme.palette.textSecondary);
        ImGui::TextUnformatted(detail);
        ImGui::PopStyleColor();
        theme.PopFont();
    }
}

void Card::SameLine(float spacing) {
    const UiTheme& theme = UiTheme::Get();
    if (spacing >= 0.0f) {
        ImGui::SameLine(0.0f, theme.Scale(spacing));
    } else {
        ImGui::SameLine();
    }
}

void Card::RightAlign(float itemWidth) {
    float avail = ImGui::GetContentRegionAvail().x;
    if (avail > itemWidth) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - itemWidth));
    }
}

void Card::Spacing() {
    ImGui::Spacing();
}

void Card::Separator() {
    ImGui::Separator();
}

void Card::AlignTextToFrame() {
    ImGui::AlignTextToFramePadding();
}

void Card::AlignTextToButton(UiSize size) {
    const UiTheme& theme = UiTheme::Get();
    ControlMetrics m = theme.GetMetrics(size);
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!window || window->SkipItems) return;

    float fontSize = theme.Scale(m.fontSize);
    float offset = std::max(0.0f, std::floor((m.height - fontSize) * 0.5f));
    window->DC.CurrLineTextBaseOffset = ImMax(window->DC.CurrLineTextBaseOffset, offset);
    window->DC.CurrLineSize.y = ImMax(window->DC.CurrLineSize.y, m.height);
}
