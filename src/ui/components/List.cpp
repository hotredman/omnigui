#include "ui/components/List.hpp"
#include <imgui.h>
#include <algorithm>

List::List(const ListOptions& options) {
    const UiTheme& theme = UiTheme::Get();
    m_style = options.style ? options.style : &theme.list;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(2.0f), theme.Scale(2.0f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, theme.Scale(m_style->itemSpacing)));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(m_style->colBg).Value);

    const ImVec2 size(options.width > 0.0f ? theme.Scale(options.width) : 0.0f,
                      options.height > 0.0f ? theme.Scale(options.height) : 0.0f);
    m_open = ImGui::BeginChild(options.key ? options.key : "##List", size, false, flags);
}

List::~List() {
    // EndChild обязателен независимо от результата BeginChild
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

bool List::Header(const char* title, const ListHeaderOptions& options) {
    const char* actionIcon = options.actionIcon;
    const char* badgeText = options.badge;
    bool actionClicked = false;
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& st = m_style ? *m_style : theme.list;

    ImFont* font = st.headerFont ? st.headerFont : theme.fontBold;
    theme.PushFont(font, st.headerFontSize);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    float lineHeight = ImGui::GetTextLineHeight();
    float headerH = lineHeight + theme.Scale(8.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float padX = theme.Scale(st.paddingX);

    // Рисуем заголовок
    dl->AddText(ImVec2(pos.x + padX, pos.y + theme.Scale(4.0f)), st.colHeader, title);

    float rightPos = pos.x + availW - padX;

    // Опциональная кнопка действия справа (например, "+")
    if (actionIcon && actionIcon[0] != '\0') {
        float btnSize = theme.Scale(22.0f);
        rightPos -= btnSize;
        ImGui::SetCursorScreenPos(ImVec2(rightPos, pos.y + (headerH - btnSize) * 0.5f));
        ImGui::PushID(title);
        if (ImGui::SmallButton(actionIcon)) {
            actionClicked = true;
        }
        ImGui::PopID();
    }

    // Опциональный бейдж с количеством (например, "3")
    if (badgeText && badgeText[0] != '\0') {
        ImVec2 badgeSz = ImGui::CalcTextSize(badgeText);
        float badgeW = badgeSz.x + theme.Scale(10.0f);
        rightPos -= (badgeW + theme.Scale(6.0f));
        float badgeY = pos.y + (headerH - badgeSz.y - 4.0f) * 0.5f;
        dl->AddRectFilled(ImVec2(rightPos, badgeY),
                          ImVec2(rightPos + badgeW, badgeY + badgeSz.y + 4.0f),
                          theme.palette.bgControl, theme.Scale(4.0f));
        dl->AddText(ImVec2(rightPos + theme.Scale(5.0f), pos.y + (headerH - badgeSz.y) * 0.5f),
                    st.colTextMuted, badgeText);
    }

    theme.PopFont();
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + headerH));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
    return actionClicked;
}

bool List::Item(const ListItemOptions& options)
{
    const char* label = options.label ? options.label : "";
    const char* id = options.key ? options.key : label;
    const char* sublabel = options.sublabel;
    const bool isSelected = options.selected;
    const char* rightText = options.rightText;
    const Icon icon = options.icon;
    const ImU32 iconColor = options.iconColor;
    const float height = options.height > 0.0f ? UiTheme::Get().Scale(options.height) : 0.0f;
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& st = m_style ? *m_style : theme.list;

    float defaultH = (sublabel && sublabel[0] != '\0') ? 48.0f : st.itemHeight;
    float itemH = (height > 0.0f) ? height : theme.Scale(defaultH);
    float availW = ImGui::GetContentRegionAvail().x;
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::PushID(id);
    bool pressed = ImGui::InvisibleButton("##list_item_ex", ImVec2(availW, itemH));
    ImGui::PopID();
    bool isHovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float radius = theme.Scale(st.cornerRadius);

    if (isSelected) {
        dl->AddRectFilled(pos, ImVec2(pos.x + availW, pos.y + itemH), st.colItemSelectedBg, radius);
        float barW = theme.Scale(st.activeBarWidth);
        float barPadY = theme.Scale(4.0f);
        dl->AddRectFilled(ImVec2(pos.x, pos.y + barPadY),
                          ImVec2(pos.x + barW, pos.y + itemH - barPadY),
                          st.colActiveBar, barW * 0.5f);
    } else if (isHovered) {
        dl->AddRectFilled(pos, ImVec2(pos.x + availW, pos.y + itemH), st.colItemHoverBg, radius);
    }

    float padX = theme.Scale(st.paddingX);
    float curX = pos.x + padX;

    if (icon.IsValid()) {
        float icoSize = theme.Scale(st.iconSize);
        ImVec2 icoCenter(curX + icoSize * 0.5f, pos.y + itemH * 0.5f);
        ImU32 col = (iconColor != 0) ? iconColor : (isSelected ? st.colTextSelected : st.colText);
        icon.Draw(dl, icoCenter, icoSize, col);
        curX += icoSize + theme.Scale(8.0f);
    } else if (iconColor != 0) {
        float dotRadius = theme.Scale(3.5f);
        dl->AddCircleFilled(ImVec2(curX + dotRadius, pos.y + itemH * 0.5f), dotRadius, iconColor);
        curX += dotRadius * 2.0f + theme.Scale(8.0f);
    }

    float maxTextW = availW - (curX - pos.x) - padX;
    if (rightText && rightText[0] != '\0') {
        ImVec2 rSize = ImGui::CalcTextSize(rightText);
        maxTextW -= (rSize.x + theme.Scale(10.0f));
    }
    if (maxTextW < 20.0f) maxTextW = 20.0f;

    bool hasSub = (sublabel && sublabel[0] != '\0');
    float textH = hasSub ? (theme.Scale(st.itemFontSize) + theme.Scale(st.subFontSize) + theme.Scale(2.0f))
                         : theme.Scale(st.itemFontSize);
    float line1Y = pos.y + (itemH - textH) * 0.5f;

    // Заголовок строки
    ImFont* font = st.itemFont ? st.itemFont : theme.fontMedium;
    theme.PushFont(font, st.itemFontSize);
    ImU32 textCol = isSelected ? st.colTextSelected : (isHovered ? st.colText : st.colText);

    dl->PushClipRect(ImVec2(curX, pos.y), ImVec2(curX + maxTextW, pos.y + itemH), true);
    dl->AddText(ImVec2(curX, line1Y), textCol, label);
    dl->PopClipRect();
    theme.PopFont();

    // Подзаголовок строки
    if (hasSub) {
        ImFont* subFont = st.subFont ? st.subFont : theme.fontRegular;
        theme.PushFont(subFont, st.subFontSize);
        float line2Y = line1Y + theme.Scale(st.itemFontSize) + theme.Scale(2.0f);
        ImU32 subCol = isSelected ? theme.palette.textSecondary : st.colTextMuted;
        dl->PushClipRect(ImVec2(curX, pos.y), ImVec2(curX + maxTextW, pos.y + itemH), true);
        dl->AddText(ImVec2(curX, line2Y), subCol, sublabel);
        dl->PopClipRect();
        theme.PopFont();
    }

    // Текст справа
    if (rightText && rightText[0] != '\0') {
        ImFont* subFont = st.subFont ? st.subFont : theme.fontRegular;
        theme.PushFont(subFont, st.subFontSize);
        ImVec2 rSize = ImGui::CalcTextSize(rightText);
        float rX = pos.x + availW - padX - rSize.x;
        float rY = pos.y + (itemH - rSize.y) * 0.5f;
        dl->AddText(ImVec2(rX, rY), isSelected ? st.colTextSelected : st.colTextMuted, rightText);
        theme.PopFont();
    }

    return pressed;
}

void List::Empty(const char* message, const char* detail) {
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& st = m_style ? *m_style : theme.list;

    float availW = ImGui::GetContentRegionAvail().x;
    float padX = theme.Scale(st.paddingX);

    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0.0f, theme.Scale(4.0f)));

    theme.PushFont(st.subFont ? st.subFont : theme.fontRegular, st.subFontSize);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddText(ImVec2(pos.x + padX, pos.y), st.colTextMuted, message);
    if (detail && detail[0] != '\0') {
        float lineH = ImGui::GetTextLineHeight();
        dl->AddText(ImVec2(pos.x + padX, pos.y + lineH + 2.0f), theme.palette.textMuted, detail);
        ImGui::Dummy(ImVec2(availW, lineH * 2.0f + theme.Scale(8.0f)));
    } else {
        ImGui::Dummy(ImVec2(availW, ImGui::GetTextLineHeight() + theme.Scale(8.0f)));
    }
    theme.PopFont();
}

void List::Separator() {
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& st = m_style ? *m_style : theme.list;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float y = pos.y + theme.Scale(3.0f);
    dl->AddLine(ImVec2(pos.x + theme.Scale(6.0f), y),
                ImVec2(pos.x + availW - theme.Scale(6.0f), y),
                st.colBorder, 1.0f);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, y + theme.Scale(4.0f)));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
}
