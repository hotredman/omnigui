#include "ui/components/InputField.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <cctype>
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

} // namespace

bool InputField::Float(const char* id, const char* label, float& value, 
                       const char* unit, float width, const char* format,
                       float height, UiVariant variant)
{
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    float w = (width > 0.0f) ? width : theme.Scale(160.0f);
    ImGui::BeginGroup();

    RenderFieldLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float targetH = (height > 0.0f) ? height : theme.GetMetrics(UiSize::Medium).height;
    float fontSize = theme.Scale(style.valueFontSize);
    float padY = std::max(2.0f, (targetH - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);

    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    bool changed = ImGui::InputFloat(id, &value, 0.0f, 0.0f, format);

    theme.PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    RenderUnitSuffix(unit, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), style, theme);

    ImGui::EndGroup();
    return changed;
}

bool InputField::Float(const char* id, const char* label, std::optional<float>& value, 
                       const char* unit, float width, const char* format,
                       float height, UiVariant variant)
{
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    float w = (width > 0.0f) ? width : theme.Scale(160.0f);
    ImGui::BeginGroup();

    RenderFieldLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float targetH = (height > 0.0f) ? height : theme.GetMetrics(UiSize::Medium).height;
    float fontSize = theme.Scale(style.valueFontSize);
    float padY = std::max(2.0f, (targetH - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);

    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    char buf[64] = "";
    if (value.has_value()) {
        snprintf(buf, sizeof(buf), format ? format : "%.2f", *value);
    }

    bool changed = ImGui::InputTextWithHint(id, " — ", buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal);
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
            float parsed = std::strtof(buf, &end);
            if (end != buf) {
                value = parsed;
            }
        }
    }

    theme.PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    RenderUnitSuffix(unit, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), style, theme);

    ImGui::EndGroup();
    return changed;
}

bool InputField::Double(const char* id, const char* label, double& value, 
                        const char* unit, float width, const char* format,
                        float height, UiVariant variant)
{
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    float w = (width > 0.0f) ? width : theme.Scale(160.0f);
    ImGui::BeginGroup();

    RenderFieldLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float targetH = (height > 0.0f) ? height : theme.GetMetrics(UiSize::Medium).height;
    float fontSize = theme.Scale(style.valueFontSize);
    float padY = std::max(2.0f, (targetH - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);

    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    bool changed = ImGui::InputDouble(id, &value, 0.0, 0.0, format);

    theme.PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    RenderUnitSuffix(unit, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), style, theme);

    ImGui::EndGroup();
    return changed;
}

bool InputField::Double(const char* id, const char* label, std::optional<double>& value, 
                        const char* unit, float width, const char* format,
                        float height, UiVariant variant)
{
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    float w = (width > 0.0f) ? width : theme.Scale(160.0f);
    ImGui::BeginGroup();

    RenderFieldLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float targetH = (height > 0.0f) ? height : theme.GetMetrics(UiSize::Medium).height;
    float fontSize = theme.Scale(style.valueFontSize);
    float padY = std::max(2.0f, (targetH - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);

    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    char buf[64] = "";
    if (value.has_value()) {
        snprintf(buf, sizeof(buf), format ? format : "%.2f", *value);
    }

    bool changed = ImGui::InputTextWithHint(id, " — ", buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal);
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
            double parsed = std::strtod(buf, &end);
            if (end != buf) {
                value = parsed;
            }
        }
    }

    theme.PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    RenderUnitSuffix(unit, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), style, theme);

    ImGui::EndGroup();
    return changed;
}

bool InputField::Int(const char* id, const char* label, int& value, 
                     const char* unit, float width,
                     float height, UiVariant variant)
{
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    float w = (width > 0.0f) ? width : theme.Scale(160.0f);
    ImGui::BeginGroup();

    RenderFieldLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float targetH = (height > 0.0f) ? height : theme.GetMetrics(UiSize::Medium).height;
    float fontSize = theme.Scale(style.valueFontSize);
    float padY = std::max(2.0f, (targetH - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);

    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    bool changed = ImGui::InputInt(id, &value, 0, 0);

    theme.PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    RenderUnitSuffix(unit, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), style, theme);

    ImGui::EndGroup();
    return changed;
}

bool InputField::Text(const char* id, const char* label, std::string& value, 
                      const char* hint, float width, float height, UiVariant variant)
{
    const UiTheme& theme = UiTheme::Get();
    const InputFieldStyle& style = theme.input;

    float w = (width > 0.0f) ? width : theme.Scale(200.0f);
    ImGui::BeginGroup();

    RenderFieldLabel(label, w, style);
    ImGui::SetNextItemWidth(w);

    float targetH = (height > 0.0f) ? height : theme.GetMetrics(UiSize::Medium).height;
    float fontSize = theme.Scale(style.valueFontSize);
    float padY = std::max(2.0f, (targetH - fontSize) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, theme.Scale(style.frameRounding));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(theme.Scale(style.framePaddingX), padY));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.borderSize);

    ImU32 borderColor = (variant == UiVariant::Default) ? style.colBorder : theme.GetVariantStyle(variant).colBorder;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(style.colBg).Value);
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(borderColor).Value);
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(style.colText).Value);

    ImFont* valFont = style.valueFont ? style.valueFont : theme.fontMedium;
    theme.PushFont(valFont, style.valueFontSize);

    bool changed = ImGui::InputTextWithHint(id, hint ? hint : "", &value);

    theme.PopFont();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    ImGui::EndGroup();
    return changed;
}

bool InputField::Combo(const char* id, const char* label, int& currentItem, 
                       const char* const items[], int itemsCount, float width,
                       float height, UiVariant variant)
{
    return ::Combo::Render(id, label, currentItem, items, itemsCount, width, height, variant);
}

bool InputField::Combo(const char* id, const char* label, int& currentItem, 
                       const std::vector<std::string>& items, float width,
                       float height, UiVariant variant)
{
    return ::Combo::Render(id, label, currentItem, items, width, height, variant);
}

