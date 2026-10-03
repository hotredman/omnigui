#include "ui/components/Card.hpp"
#include "ui/components/DisabledScope.hpp"
#include "ui/components/Button.hpp"
#include "ui/components/InputField.hpp"
#include "ui/components/Combo.hpp"
#include "ui/components/Toggle.hpp"
#include "ui/components/ValueDisplay.hpp"
#include "ui/components/FlowLayout.hpp"
#include "ui/components/GridLayout.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cfloat>

namespace {

// Рамка поля по его состоянию
UiVariant VariantOf(FieldState state) {
    return state == FieldState::Modified ? UiVariant::Warning : UiVariant::Default;
}

}  // namespace

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
    const char* id = options.key ? options.key : ((title && title[0] != '\0') ? title : "##card");
    ImVec2 size(theme.Scale(options.width), theme.Scale(options.height));

    if (size.x <= 0.0f) {
        float gridW = GridLayout::CurrentCellWidth();
        float flowW = FlowLayout::CurrentColumnWidth();
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

    m_open = ImGui::BeginChild(id, size, childFlags, windowFlags);
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
    span = std::clamp(span, 1, 12);
    if (span == 12) {
        return GetFullWidth();
    }
    float full = GetFullWidth();
    float s = m_columnSpacing;
    float width = (span * full - (12 - span) * s) / 12.0f;
    return std::max(20.0f, static_cast<float>(std::floor(width)));
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
// Interactive Widgets: Buttons & Display
// ============================================================================

bool Card::Button(const FieldOptions& o) {
    const ::Col col = o.col.value_or(::Col::Half());
    float h = ResolveHeight(o.size, UiSize::Medium);
    float w = 0.0f;
    if (col.IsAuto()) {
        w = ButtonWidthPx({.label = o.label, .icon = o.icon}, h);
    }
    PrepareField(col, w);

    bool clicked = ::ButtonPx(ImVec2(w, h), {.label = o.label, .variant = o.variant, .icon = o.icon,
                                             .tooltip = o.tooltip, .disabled = o.disabled, .key = o.key});

    FinishField(col);
    return clicked;
}

bool Card::HoldButton(const FieldOptions& o) {
    Button(o);
    return !o.disabled && ImGui::IsItemActive();
}

void Card::Value(double value, const FieldOptions& o) {
    char buf[64];
    snprintf(buf, sizeof(buf), o.format ? o.format : "%.2f", value);
    ValueImpl(buf, o);
}

void Card::Value(const char* text, const FieldOptions& o) {
    ValueImpl(text, o);
}

void Card::Value(std::optional<double> value, const FieldOptions& o) {
    if (value.has_value()) {
        Value(*value, o);
    } else {
        ValueImpl("—", o);
    }
}

void Card::ValueImpl(const char* text, const FieldOptions& o) {
    const ::Col col = o.col.value_or(::Col::Full());
    float w = 0.0f;
    PrepareField(col, w);

    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& inStyle = theme.input;
    const bool hasLabel = o.label && o.label[0] != '\0';

    if (hasLabel) {
        ImGui::BeginGroup();
        ImFont* font = inStyle.labelFont ? inStyle.labelFont : theme.fontRegular;
        theme.PushFont(font, inStyle.labelFontSize);

        float lineHeight = ImGui::GetTextLineHeight();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->PushClipRect(pos, ImVec2(pos.x + w, pos.y + lineHeight + 2.0f), true);
        dl->AddText(pos, inStyle.colLabel, o.label);
        dl->PopClipRect();

        ImGui::Dummy(ImVec2(w, lineHeight));
        theme.PopFont();

        float spacing = theme.Scale(inStyle.labelSpacing);
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + lineHeight + spacing));
    }

    float h = ResolveHeight(o.size);
    if (h <= 0.0f) {
        h = theme.Scale(80.0f);
    }

    ValueDisplayStyle vStyle = theme.valueDisplay;
    const bool customStyle = o.variant != UiVariant::Default;
    if (customStyle) {
        const SemanticStyle& sem = theme.GetVariantStyle(o.variant);
        vStyle.colValue = sem.colText;
        vStyle.colBorder = sem.colBorder;
    }

    ::ValueDisplay(text, {.unit = o.unit, .style = customStyle ? &vStyle : nullptr, .sizePx = ImVec2(w, h)});

    if (hasLabel) {
        ImGui::EndGroup();
    }
    FinishField(col);
}

template<typename T>
bool Card::PresetGridImpl(T& value, const std::vector<T>& presets, int columns, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Full());
    float w = 0.0f;
    PrepareField(col, w);
    const UiTheme& theme = UiTheme::Get();
    float dispH = o.size ? theme.GetMetrics(*o.size).height : theme.Scale(44.0f);
    bool changed = ::PresetGrid(value, presets, {.unit = o.unit, .columns = columns,
                                                 .displayFormat = o.format ? o.format : "%.1f",
                                                 .key = o.key, .sizePx = ImVec2(w, dispH)});
    FinishField(col);
    return changed;
}

bool Card::PresetGrid(float& value, const std::vector<float>& presets, const FieldOptions& options) {
    return PresetGridImpl<float>(value, presets, options.columns, options);
}

bool Card::PresetGrid(double& value, const std::vector<double>& presets, const FieldOptions& options) {
    return PresetGridImpl<double>(value, presets, options.columns, options);
}

// ============================================================================
// Form Fields
// ============================================================================

bool Card::Float(float& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = false;
    const FieldState state = StateOf(&value, sizeof(value));
    if (state == FieldState::NoValue) {
        RenderNoValue(o, w, h);
    } else {
        DisabledScope disabledScope(o.disabled);
        changed = InputField(value, {.label = o.label, .unit = o.unit, .format = o.format ? o.format : "%.2f",
                                     .variant = VariantOf(state), .key = o.key, .sizePx = {w, h}});
    }
    FinishField(col);
    return changed;
}

bool Card::Float(std::optional<float>& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = false;
    {
        DisabledScope disabledScope(o.disabled);
        changed = InputField(value, {.label = o.label, .unit = o.unit, .format = o.format ? o.format : "%.2f",
                                     .key = o.key, .sizePx = {w, h}});
    }
    FinishField(col);
    return changed;
}

bool Card::Double(double& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = false;
    {
        DisabledScope disabledScope(o.disabled);
        changed = InputField(value, {.label = o.label, .unit = o.unit, .format = o.format ? o.format : "%.2f",
                                     .key = o.key, .sizePx = {w, h}});
    }
    FinishField(col);
    return changed;
}

bool Card::Double(std::optional<double>& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = false;
    {
        DisabledScope disabledScope(o.disabled);
        changed = InputField(value, {.label = o.label, .unit = o.unit, .format = o.format ? o.format : "%.2f",
                                     .key = o.key, .sizePx = {w, h}});
    }
    FinishField(col);
    return changed;
}

bool Card::Toggle(bool& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);

    const FieldState state = StateOf(&value, sizeof(value));
    if (state == FieldState::NoValue) {
        RenderNoValue(o, w, ResolveHeight(std::nullopt));
        FinishField(col);
        return false;
    }

    bool shouldAlignBottom = o.alignBottom.value_or(col.span < 12 && o.sublabel == nullptr);

    if (shouldAlignBottom) {
        const UiTheme& theme = UiTheme::Get();
        const InputFieldStyle& inputStyle = theme.input;

        ImFont* font = inputStyle.labelFont ? inputStyle.labelFont : theme.fontRegular;
        theme.PushFont(font, inputStyle.labelFontSize);
        float lineHeight = ImGui::GetTextLineHeight();
        theme.PopFont();

        float spacing = theme.Scale(inputStyle.labelSpacing);
        float labelOffset = lineHeight + spacing;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + labelOffset);
    }

    const UiTheme& theme = UiTheme::Get();
    float frameHeight = ResolveHeight(o.size, UiSize::Medium);

    bool changed = ::Toggle(value, {.label = o.label, .sublabel = o.sublabel, .disabled = o.disabled,
                                    .key = o.key, .sizePx = {w, frameHeight}});
    if (state == FieldState::Modified) {
        // У переключателя нет рамки-варианта: изменённое отмечаем контуром вокруг него
        const float pad = theme.Scale(3.0f);
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(ImGui::GetItemRectMin().x - pad, ImGui::GetItemRectMin().y - pad),
            ImVec2(ImGui::GetItemRectMax().x + pad, ImGui::GetItemRectMax().y + pad),
            ImGui::GetColorU32(theme.GetVariantStyle(UiVariant::Warning).colBorder),
            theme.Scale(8.0f), 0, 1.5f);
    }
    FinishField(col);
    return changed;
}

bool Card::Int(int& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = false;
    const FieldState state = StateOf(&value, sizeof(value));
    if (state == FieldState::NoValue)
        RenderNoValue(o, w, h);
    else
        changed = InputField(value, {.label = o.label, .unit = o.unit, .variant = VariantOf(state),
                                     .key = o.key, .sizePx = {w, h}});
    FinishField(col);
    return changed;
}

bool Card::Text(std::string& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = InputField(value, {.label = o.label, .hint = o.hint ? o.hint : " — ",
                                      .key = o.key, .sizePx = {w, h}});
    FinishField(col);
    return changed;
}

void Card::Display(const std::string& value, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    const char* label = o.label;
    float w = 0.0f;
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    if (col.IsAuto()) {
        float padX = theme.Scale(style.framePaddingX);
        ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
        theme.PushFont(valFont, style.valueFontSize);
        float textW = ImGui::CalcTextSize(value.c_str()).x;
        theme.PopFont();

        float labelW = 0.0f;
        if (label && label[0] != '\0') {
            ImFont* font = style.labelFont ? style.labelFont : theme.fontRegular;
            theme.PushFont(font, style.labelFontSize);
            labelW = ImGui::CalcTextSize(label).x;
            theme.PopFont();
        }

        w = std::max(textW + padX * 2.0f + 16.0f, labelW + padX * 2.0f);
        w = std::max(w, 80.0f);
    }

    PrepareField(col, w);

    ImGui::PushID(o.key ? o.key : (label ? label : "display"));
    ImGui::BeginGroup();

    // Заголовок (Label) над полем
    if (label && label[0] != '\0') {
        ImFont* font = style.labelFont ? style.labelFont : theme.fontRegular;
        theme.PushFont(font, style.labelFontSize);

        float lineHeight = ImGui::GetTextLineHeight();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->PushClipRect(pos, ImVec2(pos.x + w, pos.y + lineHeight + 2.0f), true);
        dl->AddText(pos, style.colLabel, label);
        dl->PopClipRect();

        ImGui::Dummy(ImVec2(w, lineHeight));
        theme.PopFont();

        float spacing = theme.Scale(style.labelSpacing);
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + lineHeight + spacing));
    }

    // Рамка только для чтения
    float targetH = ResolveHeight(o.size);
    float frameH = (targetH > 0.0f) ? targetH : theme.GetMetrics(UiSize::Medium).height;
    ImVec2 minPos = ImGui::GetCursorScreenPos();
    ImVec2 maxPos(minPos.x + w, minPos.y + frameH);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(minPos, maxPos, style.colBg, theme.Scale(style.frameRounding));
    if (style.borderSize > 0.0f) {
        dl->AddRect(minPos, maxPos, style.colBorder, theme.Scale(style.frameRounding), 0, style.borderSize);
    }

    // Текст значения
    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    float fontSize = theme.Scale(style.valueFontSize);
    float textY = minPos.y + (frameH - fontSize) * 0.5f;
    float padX = theme.Scale(style.framePaddingX);

    float textW = ImGui::CalcTextSize(value.c_str()).x;
    float textX = (label && label[0] != '\0') ? (minPos.x + padX) : (minPos.x + std::max(padX, (w - textW) * 0.5f));

    dl->PushClipRect(ImVec2(minPos.x + padX, minPos.y), ImVec2(maxPos.x - padX, maxPos.y), true);
    dl->AddText(ImVec2(textX, textY), style.colText, value.c_str());
    dl->PopClipRect();

    theme.PopFont();

    ImGui::Dummy(ImVec2(w, frameH));
    ImGui::EndGroup();
    ImGui::PopID();

    FinishField(col);
}

bool Card::Combo(int& currentItem, const char* const items[], int itemsCount, const FieldOptions& o)
{
    return ComboImpl(StateOf(&currentItem, sizeof(currentItem)), currentItem, items, itemsCount, o);
}

bool Card::ComboImpl(FieldState state, int& currentItem, const char* const items[], int itemsCount,
                     const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = false;
    if (state == FieldState::NoValue)
        RenderNoValue(o, w, h);
    else
        changed = ::Combo(currentItem, items, itemsCount, {.label = o.label, .variant = VariantOf(state),
                                                           .key = o.key, .sizePx = {w, h}});
    FinishField(col);
    return changed;
}

bool Card::Combo(int& currentItem, const std::vector<std::string>& items, const FieldOptions& o)
{
    const ::Col col = o.col.value_or(::Col::Half());
    float w = 0.0f;
    PrepareField(col, w);
    float h = ResolveHeight(o.size);
    bool changed = false;
    const FieldState state = StateOf(&currentItem, sizeof(currentItem));
    if (state == FieldState::NoValue)
        RenderNoValue(o, w, h);
    else
        changed = ::Combo(currentItem, items, {.label = o.label, .variant = VariantOf(state),
                                               .key = o.key, .sizePx = {w, h}});
    FinishField(col);
    return changed;
}

FieldState Card::StateOf(const void* field, std::size_t size) const {
    return m_fieldStates ? m_fieldStates(field, size) : FieldState::Normal;
}

void Card::RenderNoValue(const FieldOptions& o, float width, float height) {
    DisabledScope off(true);
    std::string dash = "—";
    InputField(dash, {.label = o.label ? o.label : "", .hint = " — ", .key = o.key, .sizePx = {width, height}});
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
