#include "ui/components/ToolButton.hpp"
#include "ui/components/DisabledScope.hpp"
#include <algorithm>
#include <cmath>
#include <string>

static void ApplyToolbarVerticalCentering(float s) {
    const UiTheme& theme = UiTheme::Get();
    float winH = ImGui::GetWindowHeight();
    // В компактных полосах (тулбар, шапка) центрируем кнопку по вертикали, предотвращая растягивание
    if (winH <= theme.HeaderHeight() * 1.15f) {
        float targetY = std::max(0.0f, std::floor((winH - s) * 0.5f));
        if (ImGui::GetCursorPosY() < targetY) {
            ImGui::SetCursorPosY(targetY);
        }
    }
}

static void ResolveButtonColors(const UiTheme& theme, UiVariant variant, bool hovered, bool active, bool selected,
                               ImU32& outBg, ImU32& outBorder, ImU32& outForeground) {
    if (variant == UiVariant::Default) {
        if (selected) {
            outBg = theme.toolButton.colBgSelected;
            outBorder = theme.toolButton.colBorderSelected;
            outForeground = theme.toolButton.colIconHover;
        } else if (active) {
            outBg = theme.toolButton.colBgActive;
            outBorder = theme.toolButton.colBorderHover;
            outForeground = theme.toolButton.colIconHover;
        } else if (hovered) {
            outBg = theme.toolButton.colBgHover;
            outBorder = theme.toolButton.colBorderHover;
            outForeground = theme.toolButton.colIconHover;
        } else {
            outBg = theme.toolButton.colBg;
            outBorder = theme.toolButton.colBorder;
            outForeground = theme.toolButton.colIcon;
        }
    } else {
        const SemanticStyle& st = theme.GetVariantStyle(variant);
        ImU32 baseBg = ImGui::ColorConvertFloat4ToU32(st.colBg);
        ImU32 hoverBg = ImGui::ColorConvertFloat4ToU32(st.colBgHover);
        ImU32 activeBg = ImGui::ColorConvertFloat4ToU32(st.colBgActive);

        if (active) {
            outBg = activeBg;
            outBorder = st.colBorder;
            outForeground = st.colBtnText;
        } else if (hovered) {
            outBg = hoverBg;
            outBorder = st.colBorder;
            outForeground = st.colBtnText;
        } else {
            outBg = baseBg;
            outBorder = st.colBorder;
            outForeground = st.colBtnText;
        }
    }
}

bool ToolButton(const ToolButtonOptions& o) {
    DisabledScope disabledScope(o.disabled);
    const UiTheme& theme = UiTheme::Get();

    // Сторона квадрата: sidePx (контейнеры) > size > размер из темы
    float s;
    if (o.sidePx > 0.0f)       s = o.sidePx;
    else if (o.size)           s = theme.GetMetrics(*o.size).height;
    else                       s = theme.Scale(theme.toolButton.size);

    ApplyToolbarVerticalCentering(s);

    // Идентичность: явный key, иначе глиф, иначе иконка
    if (o.key)         ImGui::PushID(o.key);
    else if (o.glyph)  ImGui::PushID(o.glyph);
    else               ImGui::PushID(static_cast<int>(o.icon.GetId()));

    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton("##tool", ImVec2(s, s));
    ImGui::PopID();
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    ImU32 bgCol = 0, borderCol = 0, fgCol = 0;
    ResolveButtonColors(theme, o.variant, hovered, active, o.selected, bgCol, borderCol, fgCol);

    if (o.iconColor != 0 && !hovered && !active && !o.selected) {
        fgCol = o.iconColor;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float radius = theme.Scale(theme.toolButton.cornerRadius);

    // Подложка и контур
    dl->AddRectFilled(screenPos, ImVec2(screenPos.x + s, screenPos.y + s), ImGui::GetColorU32(bgCol), radius);
    if (theme.toolButton.borderSize > 0.0f) {
        dl->AddRect(screenPos, ImVec2(screenPos.x + s, screenPos.y + s), ImGui::GetColorU32(borderCol), radius, 0, theme.toolButton.borderSize);
    }

    const float pressOffset = active ? 1.0f : 0.0f;

    // Векторная пиктограмма
    if (o.icon != Icon::None) {
        ImVec2 center(screenPos.x + s * 0.5f + pressOffset, screenPos.y + s * 0.5f + pressOffset);
        float iconSz = s * theme.toolButton.iconScale;
        o.icon.Draw(dl, center, iconSz, ImGui::GetColorU32(fgCol));
    }

    // Текстовый символ / глиф
    if (o.glyph && o.glyph[0] != '\0') {
        ImFont* font = theme.toolButton.font ? theme.toolButton.font : (theme.buttonFont ? theme.buttonFont : theme.defaultFont);
        float fontSz = theme.Scale(theme.toolButton.fontSize);
        if (font) {
            ImVec2 textSize = font->CalcTextSizeA(fontSz, FLT_MAX, 0.0f, o.glyph);
            ImVec2 textPos(screenPos.x + (s - textSize.x) * 0.5f + pressOffset,
                           screenPos.y + (s - textSize.y) * 0.5f + pressOffset);
            dl->AddText(font, fontSz, textPos, ImGui::GetColorU32(fgCol), o.glyph);
        } else {
            ImVec2 textSize = ImGui::CalcTextSize(o.glyph);
            ImVec2 textPos(screenPos.x + (s - textSize.x) * 0.5f + pressOffset,
                           screenPos.y + (s - textSize.y) * 0.5f + pressOffset);
            dl->AddText(textPos, ImGui::GetColorU32(fgCol), o.glyph);
        }
    }

    // Всплывающая подсказка
    if (o.tooltip && o.tooltip[0] != '\0' && hovered) {
        ImGui::SetTooltip("%s", o.tooltip);
    }

    return clicked;
}