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

bool ToolButton::Render(const char* id, Icon icon, UiVariant variant, const char* tooltip, bool selected, float baseSize, ImU32 iconColor, bool disabled) {
    DisabledScope disabledScope(disabled);
    const UiTheme& theme = UiTheme::Get();
    float s = (baseSize > 0.0f) ? theme.Scale(baseSize) : theme.Scale(theme.toolButton.size);

    ApplyToolbarVerticalCentering(s);

    char safeId[64];
    const char* targetId = id;
    if (!id || id[0] != '#' || id[1] != '#') {
        std::snprintf(safeId, sizeof(safeId), "##tb_icon_%s", id ? id : "btn");
        targetId = safeId;
    }
    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(targetId, ImVec2(s, s));
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    ImU32 bgCol = 0, borderCol = 0, iconCol = 0;
    ResolveButtonColors(theme, variant, hovered, active, selected, bgCol, borderCol, iconCol);

    if (iconColor != 0 && !hovered && !active && !selected) {
        iconCol = iconColor;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float radius = theme.Scale(theme.toolButton.cornerRadius);

    // Подложка и контур
    dl->AddRectFilled(screenPos, ImVec2(screenPos.x + s, screenPos.y + s), ImGui::GetColorU32(bgCol), radius);
    if (theme.toolButton.borderSize > 0.0f) {
        dl->AddRect(screenPos, ImVec2(screenPos.x + s, screenPos.y + s), ImGui::GetColorU32(borderCol), radius, 0, theme.toolButton.borderSize);
    }

    // Векторная пиктограмма
    if (icon != Icon::None) {
        float pressOffset = active ? 1.0f : 0.0f;
        ImVec2 center(screenPos.x + s * 0.5f + pressOffset, screenPos.y + s * 0.5f + pressOffset);
        float iconSz = s * theme.toolButton.iconScale;
        icon.Draw(dl, center, iconSz, ImGui::GetColorU32(iconCol));
    }

    // Всплывающая подсказка
    if (tooltip && tooltip[0] != '\0' && hovered) {
        ImGui::SetTooltip("%s", tooltip);
    }

    return clicked;
}

bool ToolButton::Render(const char* id, Icon icon, UiSize size, UiVariant variant, const char* tooltip, bool selected, ImU32 iconColor, bool disabled) {
    const UiTheme& theme = UiTheme::Get();
    ControlMetrics m = theme.GetMetrics(size);
    float baseSize = m.height / theme.GetScale();
    return Render(id, icon, variant, tooltip, selected, baseSize, iconColor, disabled);
}

bool ToolButton::Render(const char* id, const char* glyphOrLabel, UiVariant variant, const char* tooltip, bool selected, float baseSize, bool disabled) {
    DisabledScope disabledScope(disabled);
    const UiTheme& theme = UiTheme::Get();
    float s = (baseSize > 0.0f) ? theme.Scale(baseSize) : theme.Scale(theme.toolButton.size);

    ApplyToolbarVerticalCentering(s);

    char safeId[64];
    const char* targetId = id;
    if (!id || id[0] != '#' || id[1] != '#') {
        std::snprintf(safeId, sizeof(safeId), "##tb_txt_%s", id ? id : (glyphOrLabel ? glyphOrLabel : "btn"));
        targetId = safeId;
    }
    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(targetId, ImVec2(s, s));
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    ImU32 bgCol = 0, borderCol = 0, textCol = 0;
    ResolveButtonColors(theme, variant, hovered, active, selected, bgCol, borderCol, textCol);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float radius = theme.Scale(theme.toolButton.cornerRadius);

    // Подложка и контур
    dl->AddRectFilled(screenPos, ImVec2(screenPos.x + s, screenPos.y + s), ImGui::GetColorU32(bgCol), radius);
    if (theme.toolButton.borderSize > 0.0f) {
        dl->AddRect(screenPos, ImVec2(screenPos.x + s, screenPos.y + s), ImGui::GetColorU32(borderCol), radius, 0, theme.toolButton.borderSize);
    }

    // Текстовый символ / глиф
    if (glyphOrLabel && glyphOrLabel[0] != '\0') {
        ImFont* font = theme.toolButton.font ? theme.toolButton.font : (theme.buttonFont ? theme.buttonFont : theme.defaultFont);
        float fontSz = theme.Scale(theme.toolButton.fontSize);
        float pressOffset = active ? 1.0f : 0.0f;
        if (font) {
            ImVec2 textSize = font->CalcTextSizeA(fontSz, FLT_MAX, 0.0f, glyphOrLabel);
            ImVec2 textPos(screenPos.x + (s - textSize.x) * 0.5f + pressOffset,
                           screenPos.y + (s - textSize.y) * 0.5f + pressOffset);
            dl->AddText(font, fontSz, textPos, ImGui::GetColorU32(textCol), glyphOrLabel);
        } else {
            ImVec2 textSize = ImGui::CalcTextSize(glyphOrLabel);
            ImVec2 textPos(screenPos.x + (s - textSize.x) * 0.5f + pressOffset,
                           screenPos.y + (s - textSize.y) * 0.5f + pressOffset);
            dl->AddText(textPos, ImGui::GetColorU32(textCol), glyphOrLabel);
        }
    }

    // Всплывающая подсказка
    if (tooltip && tooltip[0] != '\0' && hovered) {
        ImGui::SetTooltip("%s", tooltip);
    }

    return clicked;
}
