#include "ui/components/Tag.hpp"
#include <imgui.h>
#include <cstdio>
#include <string>
#include <algorithm>

bool Tag::Render(const char* label, const char* prefix, bool clickable, const char* tooltip) {
    if (!label || label[0] == '\0') return false;

    const UiTheme& theme = UiTheme::Get();
    const TagStyle& st = theme.tag;

    theme.PushFont(st.font ? st.font : theme.fontMedium, st.fontSize);

    std::string prefixStr = (prefix && prefix[0] != '\0') ? (std::string(prefix) + " ") : "";
    ImVec2 prefixSz = prefixStr.empty() ? ImVec2(0.0f, 0.0f) : ImGui::CalcTextSize(prefixStr.c_str());
    ImVec2 labelSz = ImGui::CalcTextSize(label);

    float padX = theme.Scale(st.paddingX);
    float padY = theme.Scale(st.paddingY);
    float w = prefixSz.x + labelSz.x + padX * 2.0f;
    float h = std::max(prefixSz.y, labelSz.y) + padY * 2.0f;

    ImVec2 p = ImGui::GetCursorScreenPos();
    bool clicked = false;
    bool hovered = false;
    bool active = false;

    if (clickable) {
        char btnId[64];
        std::snprintf(btnId, sizeof(btnId), "##tag_%s", label);
        clicked = ImGui::InvisibleButton(btnId, ImVec2(w, h));
        hovered = ImGui::IsItemHovered();
        active = ImGui::IsItemActive();
    } else {
        ImGui::Dummy(ImVec2(w, h));
    }

    // Плашка за краем видимой области не рисуется (как встроенные виджеты)
    if (!ImGui::IsItemVisible()) {
        theme.PopFont();
        return clicked;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImU32 bgCol = active ? st.colBgActive : (hovered ? st.colBgHover : st.colBg);
    ImU32 borderCol = (active || hovered) ? st.colBorderHover : st.colBorder;
    ImU32 prefixCol = hovered ? st.colPrefixHover : st.colPrefix;
    ImU32 labelCol = hovered ? st.colTextHover : st.colText;

    float rounding = theme.Scale(st.cornerRadius);
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bgCol, rounding);
    dl->AddRect(p, ImVec2(p.x + w, p.y + h), borderCol, rounding, 0, st.borderSize);

    float textX = p.x + padX;
    float textY = p.y + padY;
    if (!prefixStr.empty()) {
        dl->AddText(ImVec2(textX, textY), prefixCol, prefixStr.c_str());
        textX += prefixSz.x;
    }
    dl->AddText(ImVec2(textX, textY), labelCol, label);

    theme.PopFont();

    if (clickable && hovered) {
        if (tooltip && tooltip[0] != '\0') {
            ImGui::SetTooltip("%s", tooltip);
        } else {
            ImGui::SetTooltip("Filter by tag #%s", label);
        }
    }

    return clicked;
}

bool Tag::RenderList(const std::vector<std::string>& tags, float maxAvailableWidth,
                     std::string* outClickedTag) {
    if (tags.empty()) return false;

    const UiTheme& theme = UiTheme::Get();
    const TagStyle& st = theme.tag;
    float itemSpacingX = theme.Scale(st.spacingX);
    float itemSpacingY = theme.Scale(st.spacingY);
    float availW = (maxAvailableWidth > 0.0f) ? maxAvailableWidth : ImGui::GetContentRegionAvail().x;
    float startX = ImGui::GetCursorPosX();
    float curLineW = 0.0f;
    bool anyClicked = false;

    theme.PushFont(st.font ? st.font : theme.fontMedium, st.fontSize);
    for (size_t i = 0; i < tags.size(); ++i) {
        const std::string& tag = tags[i];
        std::string fullText = "# " + tag;
        ImVec2 textSz = ImGui::CalcTextSize(fullText.c_str());
        float tagW = textSz.x + (theme.Scale(st.paddingX) * 2.0f);

        if (i > 0) {
            if (curLineW + itemSpacingX + tagW <= availW) {
                ImGui::SameLine(0.0f, itemSpacingX);
                curLineW += itemSpacingX;
            } else {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + itemSpacingY);
                ImGui::SetCursorPosX(startX);
                curLineW = 0.0f;
            }
        }
        curLineW += tagW;

        theme.PopFont();
        if (Tag::Render(tag.c_str(), "#", true)) {
            anyClicked = true;
            if (outClickedTag) *outClickedTag = tag;
        }
        theme.PushFont(st.font ? st.font : theme.fontMedium, st.fontSize);
    }
    theme.PopFont();

    return anyClicked;
}
