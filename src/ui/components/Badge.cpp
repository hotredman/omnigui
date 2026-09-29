#include "ui/components/Badge.hpp"
#include <imgui.h>
#include <cstdio>
#include <algorithm>

ImVec2 Badge::CalcSize(const char* text, float fontSize) {
    if (!text || text[0] == '\0') return ImVec2(0.0f, 0.0f);
    const UiTheme& theme = UiTheme::Get();
    const BadgeStyle& bst = theme.badge;
    float scale = theme.GetScale();
    float resolvedFontSize = (fontSize > 0.0f) ? fontSize : bst.fontSize;
    theme.PushFont(bst.font ? bst.font : theme.fontMedium, resolvedFontSize);
    ImVec2 textSz = ImGui::CalcTextSize(text);
    theme.PopFont();
    float padX = bst.paddingX * scale;
    float padY = bst.paddingY * scale;
    return ImVec2(textSz.x + padX * 2.0f, textSz.y + padY * 2.0f);
}

ImVec2 Badge::Draw(ImDrawList* dl, ImVec2 pos, const char* text, UiVariant variant, float fontSize) {
    if (!text || text[0] == '\0') return ImVec2(0.0f, 0.0f);

    const UiTheme& theme = UiTheme::Get();
    const BadgeStyle& bst = theme.badge;
    float scale = theme.GetScale();
    float resolvedFontSize = (fontSize > 0.0f) ? fontSize : bst.fontSize;
    theme.PushFont(bst.font ? bst.font : theme.fontMedium, resolvedFontSize);
    ImVec2 textSz = ImGui::CalcTextSize(text);
    float padX = bst.paddingX * scale;
    float padY = bst.paddingY * scale;
    float w = textSz.x + padX * 2.0f;
    float h = textSz.y + padY * 2.0f;

    // За краем видимой области не рисуется; размер нужен раскладке всё равно
    if (!ImGui::IsRectVisible(pos, ImVec2(pos.x + w, pos.y + h))) {
        theme.PopFont();
        return ImVec2(w, h);
    }

    const BadgeVariantColors& colors = bst.GetColors(variant);
    float rounding = bst.cornerRadius * scale;

    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), colors.colBg, rounding);
    dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), colors.colBorder, rounding, 0, bst.borderSize);
    dl->AddText(ImVec2(pos.x + padX, pos.y + padY), colors.colText, text);
    theme.PopFont();

    return ImVec2(w, h);
}

void Badge::Render(const char* text, UiVariant variant, float topOffset, float fontSize, const char* tooltip) {
    if (!text || text[0] == '\0') return;

    const UiTheme& theme = UiTheme::Get();
    if (topOffset >= 0.0f) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + theme.Scale(topOffset));
    }

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 sz = Draw(dl, p, text, variant, fontSize);
    ImGui::Dummy(sz);

    if (tooltip && tooltip[0] != '\0' && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
}

void Badge::Number(int value, const char* unit, UiVariant variant, float topOffset, float fontSize) {
    char buf[64];
    if (unit && unit[0] != '\0') {
        std::snprintf(buf, sizeof(buf), "%d %s", value, unit);
    } else {
        std::snprintf(buf, sizeof(buf), "%d", value);
    }
    Render(buf, variant, topOffset, fontSize);
}
