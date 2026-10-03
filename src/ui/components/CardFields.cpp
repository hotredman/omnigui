// Поля формы Card: кнопки, значения, числа, переключатели, текст, комбо.
// Геометрия строк и колонок, жизненный цикл карточки — в Card.cpp.
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

namespace {

// Рамка поля по его состоянию
UiVariant VariantOf(FieldState state) {
    return state == FieldState::Modified ? UiVariant::Warning : UiVariant::Default;
}

}  // namespace

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
                                                 .size = o.size.value_or(UiSize::Medium),
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

