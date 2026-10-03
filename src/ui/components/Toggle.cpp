#include "ui/components/Toggle.hpp"
#include "ui/components/DisabledScope.hpp"
#include "imgui.h"
#include <algorithm>

bool Toggle(bool& value, const ToggleOptions& options, std::source_location loc)
{
    const char* label = options.label;
    const char* sublabel = options.sublabel;

    DisabledScope disabledScope(options.disabled);
    UiTheme& theme = UiTheme::Get();
    const ToggleStyle& style = options.style ? *options.style : theme.toggle;

    // Геометрия: sizePx (контейнеры) перекрывает width; высота 0 — по стилю
    const float width = (options.sizePx.x > 0.0f) ? options.sizePx.x
                      : (options.width > 0.0f ? theme.Scale(options.width) : 0.0f);
    const float height = options.sizePx.y;
    float scale = theme.GetScale();
    const float sizeFactor = theme.SizeFactor(options.size);
    const float fontSizeLabel = style.fontSizeLabel * sizeFactor;
    const float fontSizeSublabel = style.fontSizeSublabel * sizeFactor;

    // 1. Опорные размеры с учётом масштабирования темы
    float switchW = style.switchWidth * scale * sizeFactor;
    float switchH = style.switchHeight * scale * sizeFactor;
    float knobPad = style.knobPadding * scale * sizeFactor;
    float labelSpacing = style.labelSpacing * scale * sizeFactor;
    float minH = (height > 0.0f) ? height : (style.minHeight * scale * sizeFactor);

    ImFont* fontMain = style.fontLabel ? style.fontLabel : theme.fontMedium;
    ImFont* fontSub  = style.fontSublabel ? style.fontSublabel : theme.fontRegular;

    // 2. Расчет геометрии текста
    ImVec2 labelSize(0.0f, 0.0f);
    ImVec2 sublabelSize(0.0f, 0.0f);

    if (label && label[0] != '\0') {
        theme.PushFont(fontMain, fontSizeLabel);
        labelSize = ImGui::CalcTextSize(label);
        theme.PopFont();
    }

    if (sublabel && sublabel[0] != '\0') {
        theme.PushFont(fontSub, fontSizeSublabel);
        sublabelSize = ImGui::CalcTextSize(sublabel);
        theme.PopFont();
    }

    float totalTextH = 0.0f;
    if (labelSize.y > 0.0f && sublabelSize.y > 0.0f) {
        totalTextH = labelSize.y + style.textLineSpacing * scale * sizeFactor + sublabelSize.y;
    } else if (labelSize.y > 0.0f) {
        totalTextH = labelSize.y;
    } else if (sublabelSize.y > 0.0f) {
        totalTextH = sublabelSize.y;
    }

    float totalContentW = switchW;
    float maxTextW = std::max(labelSize.x, sublabelSize.x);
    if (maxTextW > 0.0f) {
        totalContentW += labelSpacing + maxTextW;
    }

    float finalW = (width > 0.0f) ? width : totalContentW;
    float finalH = std::max(minH, std::max(switchH, totalTextH));

    // 3. Интерактивная область (InvisibleButton)
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    // Идентичность: явный key, иначе loc + label
    {
        AutoIdScope idScope(options.key, label, loc);
        ImGui::InvisibleButton("##toggle", ImVec2(finalW, finalH));
    }

    bool isHovered = ImGui::IsItemHovered();
    bool isClicked = ImGui::IsItemClicked();

    if (isHovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    bool changed = false;
    if (isClicked) {
        value = !value;
        changed = true;
    }

    // 4. Отрисовка
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Вертикальное центрирование тумблера
    float switchY = cursorPos.y + (finalH - switchH) * 0.5f;
    ImVec2 trackMin(cursorPos.x, switchY);
    ImVec2 trackMax(cursorPos.x + switchW, switchY + switchH);
    float trackRounding = switchH * 0.5f;

    // Выбор цветов трека (выключенный вид — приглушение DisabledScope)
    ImU32 colTrack;
    ImU32 colBorder;
    if (value) {
        colTrack = isHovered ? style.colTrackOnHover : style.colTrackOn;
        colBorder = isHovered ? style.colTrackOnHover : style.colTrackOnBorder;
    } else {
        colTrack = isHovered ? style.colTrackOffHover : style.colTrackOff;
        colBorder = isHovered ? style.colTrackOffBorder : style.colTrackOffBorder;
    }

    drawList->AddRectFilled(trackMin, trackMax, ImGui::GetColorU32(colTrack), trackRounding);
    drawList->AddRect(trackMin, trackMax, ImGui::GetColorU32(colBorder), trackRounding, 0, 1.0f);

    // Круглый бегунок (Knob)
    float knobRadius = (switchH - 2.0f * knobPad) * 0.5f;
    float knobXOff = trackMin.x + knobPad + knobRadius;
    float knobXOn  = trackMax.x - knobPad - knobRadius;
    float knobCenterX = value ? knobXOn : knobXOff;
    float knobCenterY = switchY + switchH * 0.5f;

    ImU32 colKnob = value ? style.colKnobOn : style.colKnobOff;
    drawList->AddCircleFilled(ImVec2(knobCenterX, knobCenterY), knobRadius, ImGui::GetColorU32(colKnob), 16);

    // 5. Отрисовка текста (Label + Sublabel)
    if (maxTextW > 0.0f) {
        float textX = cursorPos.x + switchW + labelSpacing;
        float textY = cursorPos.y + (finalH - totalTextH) * 0.5f;

        if (label && label[0] != '\0') {
            theme.PushFont(fontMain, fontSizeLabel);
            drawList->AddText(ImVec2(textX, textY), ImGui::GetColorU32(style.colTextMain), label);
            theme.PopFont();
            textY += labelSize.y + style.textLineSpacing * scale * sizeFactor;
        }

        if (sublabel && sublabel[0] != '\0') {
            theme.PushFont(fontSub, fontSizeSublabel);
            drawList->AddText(ImVec2(textX, textY), ImGui::GetColorU32(style.colTextSub), sublabel);
            theme.PopFont();
        }
    }

    return changed;
}
