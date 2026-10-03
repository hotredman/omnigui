#include "ui/components/InputField.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>

namespace {

void RenderFieldLabel(const char* label, float width, const InputFieldStyle& style) {
    if (!label || label[0] == '\0') {
        return;
    }

    const UiTheme& theme = UiTheme::Get();
    ImFont* font = style.labelFont ? style.labelFont : theme.fontRegular;
    theme.PushFont(font, style.labelFontSize);

    float lineHeight = ImGui::GetTextLineHeight();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Клипируем строго по ширине width, чтобы длинный лейбл никогда не вылезал на соседнее поле
    dl->PushClipRect(pos, ImVec2(pos.x + width, pos.y + lineHeight + 2.0f), true);
    dl->AddText(pos, ImGui::GetColorU32(style.colLabel), label);
    dl->PopClipRect();

    // Гарантируем, что габарит лейбла в лейауте занимает строго ширину width
    ImGui::Dummy(ImVec2(width, lineHeight));

    theme.PopFont();

    // Точная установка позиции курсора: поле ввода начнется ровно через labelSpacing пикселей
    float spacing = theme.Scale(style.labelSpacing);
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + lineHeight + spacing));
}

void RenderUnitSuffix(const char* unit, const ImVec2& itemMin, const ImVec2& itemMax,
                      const InputFieldStyle& style, const UiTheme& theme)
{
    if (!unit || unit[0] == '\0') {
        return;
    }

    ImFont* unitFont = style.unitFont ? style.unitFont : theme.fontRegular;
    theme.PushFont(unitFont, style.unitFontSize);

    ImVec2 unitSize = ImGui::CalcTextSize(unit);
    float padRight = theme.Scale(8.0f);
    ImVec2 unitPos(itemMax.x - unitSize.x - padRight, itemMin.y + (itemMax.y - itemMin.y - unitSize.y) * 0.5f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float bgMinX = std::max(itemMin.x + style.borderSize, unitPos.x - theme.Scale(6.0f));
    float bgMaxX = itemMax.x - style.borderSize;
    float bgMinY = itemMin.y + style.borderSize;
    float bgMaxY = itemMax.y - style.borderSize;
    float rounding = std::max(0.0f, theme.Scale(style.frameRounding) - style.borderSize);

    // Подложка под единицу измерения предотвращает наложение длинных чисел при горизонтальной прокрутке
    dl->AddRectFilled(ImVec2(bgMinX, bgMinY), ImVec2(bgMaxX, bgMaxY), ImGui::GetColorU32(style.colBg), rounding, ImDrawFlags_RoundCornersRight);
    dl->AddText(unitPos, ImGui::GetColorU32(style.colUnit), unit);

    theme.PopFont();
}

// Общий каркас поля: подпись, стили рамки/шрифта, единица измерения.
// draw рисует сам ImGui-виджет с идентификатором "##v" и возвращает true при изменении.
template <typename DrawFn>
bool RunField(const InputOptions& o, float defaultWidthBase, DrawFn&& draw)
{
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    float w;
    if (o.sizePx.x > 0.0f)                   w = o.sizePx.x;
    else if (o.width == InputOptions::Fill)  w = ImGui::GetContentRegionAvail().x;
    else if (o.width > 0.0f)                 w = theme.Scale(o.width);
    else                                     w = theme.Scale(defaultWidthBase);

    // Идентичность: явный key, иначе подпись
    ImGui::PushID(o.key ? o.key : (o.label ? o.label : "input"));
    ImGui::BeginGroup();

    RenderFieldLabel(o.label, w, style);
    ImGui::SetNextItemWidth(w);

    float targetH = (o.sizePx.y > 0.0f) ? o.sizePx.y : theme.GetMetrics(o.size).height;
    float fontSize = theme.Scale(style.valueFontSize);
    float padY = std::max(2.0f, (targetH - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);

    ImU32 borderColor = (o.variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(o.variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);

    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    bool changed = draw();

    theme.PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    RenderUnitSuffix(o.unit, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), style, theme);

    ImGui::EndGroup();
    ImGui::PopID();
    return changed;
}

// Опциональное число: пустая строка -> nullopt, иначе разбор strtof/strtod
template <typename T, typename ParseFn>
bool OptionalNumber(std::optional<T>& value, const InputOptions& o, ParseFn parse)
{
    return RunField(o, 160.0f, [&] {
        char buf[64] = "";
        if (value.has_value()) {
            snprintf(buf, sizeof(buf), o.format ? o.format : "%.2f", *value);
        }

        bool changed = ImGui::InputTextWithHint("##v", o.hint ? o.hint : "", buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal);
        if (changed) {
            bool hasDigits = false;
            for (int i = 0; buf[i] != '\0'; ++i) {
                if (!std::isspace(static_cast<unsigned char>(buf[i]))) {
                    hasDigits = true;
                    break;
                }
            }
            if (!hasDigits) {
                value = std::nullopt;
            } else {
                char* end = nullptr;
                T parsed = parse(buf, &end);
                if (end != buf) {
                    value = parsed;
                }
            }
        }
        return changed;
    });
}

} // namespace

bool InputField(float& value, const InputOptions& o) {
    return RunField(o, 160.0f, [&] { return ImGui::InputFloat("##v", &value, 0.0f, 0.0f, o.format); });
}

bool InputField(double& value, const InputOptions& o) {
    return RunField(o, 160.0f, [&] { return ImGui::InputDouble("##v", &value, 0.0, 0.0, o.format); });
}

bool InputField(int& value, const InputOptions& o) {
    return RunField(o, 160.0f, [&] { return ImGui::InputInt("##v", &value, 0, 0); });
}

bool InputField(std::string& value, const InputOptions& o) {
    return RunField(o, 200.0f, [&] { return ImGui::InputTextWithHint("##v", o.hint ? o.hint : "", &value); });
}

bool InputField(std::optional<float>& value, const InputOptions& o) {
    return OptionalNumber(value, o, [](const char* s, char** end) { return std::strtof(s, end); });
}

bool InputField(std::optional<double>& value, const InputOptions& o) {
    return OptionalNumber(value, o, [](const char* s, char** end) { return std::strtod(s, end); });
}
