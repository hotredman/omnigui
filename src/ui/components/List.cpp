#include "ui/components/List.hpp"
#include <imgui.h>
#include <algorithm>

List::List() {
}

List::~List() {
    if (m_childActive) {
        End();
    }
}

bool List::Begin(const char* id, float width, float height, const ListStyle* customStyle) {
    const UiTheme& theme = UiTheme::Get();
    m_style = customStyle ? customStyle : &theme.list;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(theme.Scale(2.0f), theme.Scale(2.0f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, theme.Scale(m_style->itemSpacing)));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(m_style->colBg).Value);

    m_childActive = true;
    bool isVisible = ImGui::BeginChild(id, ImVec2(width, height), false, flags);
    return isVisible;
}

void List::End() {
    if (m_childActive) {
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        m_childActive = false;
        m_style = nullptr;
    }
}

void List::Header(const char* title, const char* actionIcon, bool* outActionClicked, const char* badgeText) {
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
            if (outActionClicked) *outActionClicked = true;
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
}

bool List::Item(const char* id,
                const char* label,
                bool isSelected,
                const char* rightText,
                Icon icon,
                ImU32 iconColor,
                float height)
{
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& st = m_style ? *m_style : theme.list;

    float itemH = (height > 0.0f) ? height : theme.Scale(st.itemHeight);
    float availW = ImGui::GetContentRegionAvail().x;
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Невидимая кнопка для захвата ввода
    ImGui::PushID(id);
    bool pressed = ImGui::InvisibleButton("##list_item", ImVec2(availW, itemH));
    ImGui::PopID();
    bool isHovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float radius = theme.Scale(st.cornerRadius);

    // 1. Подложка выделения или наведения
    if (isSelected) {
        dl->AddRectFilled(pos, ImVec2(pos.x + availW, pos.y + itemH), st.colItemSelectedBg, radius);

        // Акцентная полоска слева
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

    // 2. Иконка или цветная LED точка статуса
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

    // 3. Текст элемента
    ImFont* font = st.itemFont ? st.itemFont : theme.fontMedium;
    theme.PushFont(font, st.itemFontSize);

    ImU32 textCol = isSelected ? st.colTextSelected : (isHovered ? st.colText : st.colText);
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    float textY = pos.y + (itemH - labelSize.y) * 0.5f;

    // Ограничиваем клипированием ширину текста, если справа есть метка
    float maxTextW = availW - (curX - pos.x) - padX;
    if (rightText && rightText[0] != '\0') {
        ImVec2 rSize = ImGui::CalcTextSize(rightText);
        maxTextW -= (rSize.x + theme.Scale(10.0f));
    }
    if (maxTextW < 20.0f) maxTextW = 20.0f;

    dl->PushClipRect(ImVec2(curX, pos.y), ImVec2(curX + maxTextW, pos.y + itemH), true);
    dl->AddText(ImVec2(curX, textY), textCol, label);
    dl->PopClipRect();

    theme.PopFont();

    // 4. Текст справа (время, метрика, доп. инфо)
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

bool List::ItemEx(const char* id,
                  const char* label,
                  const char* sublabel,
                  bool isSelected,
                  const char* rightText,
                  Icon icon,
                  ImU32 iconColor,
                  float height)
{
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
