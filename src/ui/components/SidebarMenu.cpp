#include "ui/components/SidebarMenu.hpp"
#include <algorithm>

SidebarMenu::SidebarMenu(const SidebarStyle* customStyle)
    : m_style(customStyle)
{
}

SidebarMenu::~SidebarMenu() {
    if (m_scrollRegionActive) {
        EndScrollRegion();
    }
}

bool SidebarMenu::Item(const char* id, const char* label, Icon icon, bool isSelected) {
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = m_style ? *m_style : theme.sidebar;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float itemHeight = Scale(style.itemHeight);
    float itemWidth = ImGui::GetContentRegionAvail().x;
    float cornerRadius = Scale(style.cornerRadius);

    // 1. Активный фон и правый неоново-голубой индикатор
    if (isSelected) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colActiveBg,
            cornerRadius
        );

        float barWidth = Scale(style.activeBarWidth);
        float barPad = Scale(4.0f);
        dl->AddRectFilled(
            ImVec2(cursor.x + itemWidth - barWidth, cursor.y + barPad),
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight - barPad),
            style.colActiveBar,
            barWidth * 0.5f
        );
    }

    // 2. Обработка клика и ховера
    ImGui::PushID(id);
    bool pressed = ImGui::InvisibleButton("##menu_item", ImVec2(itemWidth, itemHeight));
    ImGui::PopID();
    bool isHovered = ImGui::IsItemHovered();

    if (isHovered && !isSelected) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colHoverBg,
            cornerRadius
        );
    }

    // 3. Цвета и шрифты
    ImU32 textColor = isSelected ? style.colActiveText
                                 : (isHovered ? style.colHoverText : style.colInactiveText);

    ImFont* font = isSelected ? (style.activeFont ? style.activeFont : style.font) : style.font;
    if (!font) font = theme.defaultFont;
    float fontSize = Scale(style.fontSize);
    float textY = cursor.y + (itemHeight - fontSize) * 0.5f - 1.0f;

    // 4. Отрисовка векторной иконки
    float iconSize = Scale(style.iconSize);
    ImVec2 iconCenter(cursor.x + Scale(20.0f), cursor.y + itemHeight * 0.5f);
    icon.Draw(dl, iconCenter, iconSize, textColor);

    // 5. Отрисовка подписи с клиппированием
    float textX = cursor.x + Scale(38.0f);
    float maxTextW = itemWidth - Scale(44.0f) - (isSelected ? Scale(style.activeBarWidth + 4.0f) : 0.0f);
    if (maxTextW < 10.0f) maxTextW = 10.0f;

    dl->PushClipRect(ImVec2(textX, cursor.y), ImVec2(textX + maxTextW, cursor.y + itemHeight), true);
    dl->AddText(font, fontSize, ImVec2(textX, textY), textColor, label);
    dl->PopClipRect();

    return pressed;
}

void SidebarMenu::Separator() {
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = m_style ? *m_style : theme.sidebar;

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float itemWidth = ImGui::GetContentRegionAvail().x;
    float y = cursor.y + Scale(6.0f);
    float x1 = cursor.x + Scale(6.0f);
    float x2 = cursor.x + itemWidth - Scale(6.0f);

    dl->AddLine(ImVec2(x1, y), ImVec2(x2, y), style.colSeparator, 1.0f);
    ImGui::Dummy(ImVec2(0, Scale(12.0f)));
}

void SidebarMenu::SectionTitle(const char* title) {
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& listStyle = theme.list;

    ImFont* font = listStyle.headerFont ? listStyle.headerFont : theme.fontBold;
    theme.PushFont(font, listStyle.headerFontSize);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float padX = Scale(10.0f);
    float lineHeight = ImGui::GetTextLineHeight();
    float headerH = lineHeight + Scale(6.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddText(ImVec2(pos.x + padX, pos.y + Scale(2.0f)), listStyle.colHeader, title);

    theme.PopFont();
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + headerH));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
}

void SidebarMenu::Spacing(float height) {
    ImGui::Dummy(ImVec2(0, Scale(height)));
}

bool SidebarMenu::BeginScrollRegion(const char* id, float customHeight) {
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = m_style ? *m_style : theme.sidebar;

    float availH = (customHeight > 0.0f) ? customHeight : ImGui::GetContentRegionAvail().y;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, Scale(style.itemSpacing)));

    m_scrollRegionActive = true;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, style.colScrollRegionBg);  // подложка — только окну блока
    bool isVisible = ImGui::BeginChild(id, ImVec2(0.0f, availH), false, ImGuiWindowFlags_NoSavedSettings);
    ImGui::PopStyleColor();
    return isVisible;
}

void SidebarMenu::EndScrollRegion() {
    if (m_scrollRegionActive) {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        m_scrollRegionActive = false;
    }
}

bool SidebarMenu::ItemEx(const char* id,
                         const char* label,
                         const char* sublabel,
                         ImU32 statusColor,
                         bool isSelected)
{
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = m_style ? *m_style : theme.sidebar;
    const ListStyle& listStyle = theme.list;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float itemHeight = Scale(48.0f);
    float itemWidth = ImGui::GetContentRegionAvail().x;
    float cornerRadius = Scale(style.cornerRadius);

    // 1. Активный фон и правый индикатор
    if (isSelected) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colActiveBg,
            cornerRadius
        );

        float barWidth = Scale(style.activeBarWidth);
        float barPad = Scale(4.0f);
        dl->AddRectFilled(
            ImVec2(cursor.x + itemWidth - barWidth, cursor.y + barPad),
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight - barPad),
            style.colActiveBar,
            barWidth * 0.5f
        );
    }

    // 2. Обработка клика и ховера
    ImGui::PushID(id);
    bool pressed = ImGui::InvisibleButton("##menu_item_ex", ImVec2(itemWidth, itemHeight));
    ImGui::PopID();
    bool isHovered = ImGui::IsItemHovered();

    if (isHovered && !isSelected) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colHoverBg,
            cornerRadius
        );
    }

    // 3. Светодиодный индикатор статуса слева (LED dot)
    if (statusColor != 0) {
        float dotRadius = Scale(3.5f);
        ImVec2 dotCenter(cursor.x + Scale(18.0f), cursor.y + itemHeight * 0.5f);
        dl->AddCircleFilled(dotCenter, dotRadius, statusColor);
    }

    float textX = cursor.x + (statusColor != 0 ? Scale(30.0f) : Scale(12.0f));
    float maxTextW = itemWidth - (textX - cursor.x) - (isSelected ? Scale(style.activeBarWidth + 6.0f) : Scale(4.0f));
    if (maxTextW < 10.0f) maxTextW = 10.0f;

    bool hasSub = (sublabel && sublabel[0] != '\0');
    float textH = hasSub ? (theme.Scale(listStyle.itemFontSize) + theme.Scale(listStyle.subFontSize) + theme.Scale(2.0f))
                         : theme.Scale(listStyle.itemFontSize);
    float line1Y = cursor.y + (itemHeight - textH) * 0.5f;

    // 4. Первая строка: название образца
    ImFont* font = listStyle.itemFont ? listStyle.itemFont : theme.fontMedium;
    theme.PushFont(font, listStyle.itemFontSize);
    ImU32 textCol = isSelected ? style.colActiveText : (isHovered ? style.colHoverText : style.colInactiveText);

    dl->PushClipRect(ImVec2(textX, cursor.y), ImVec2(textX + maxTextW, cursor.y + itemHeight), true);
    dl->AddText(ImVec2(textX, line1Y), textCol, label);
    dl->PopClipRect();
    theme.PopFont();

    // 5. Вторая строка: дата и время
    if (hasSub) {
        ImFont* subFont = listStyle.subFont ? listStyle.subFont : theme.fontRegular;
        theme.PushFont(subFont, listStyle.subFontSize);
        float line2Y = line1Y + theme.Scale(listStyle.itemFontSize) + theme.Scale(2.0f);
        ImU32 subCol = isSelected ? UiTheme::Get().palette.textSecondary : style.colInactiveText;

        dl->PushClipRect(ImVec2(textX, cursor.y), ImVec2(textX + maxTextW, cursor.y + itemHeight), true);
        dl->AddText(ImVec2(textX, line2Y), subCol, sublabel);
        dl->PopClipRect();
        theme.PopFont();
    }

    return pressed;
}

void SidebarMenu::Empty(const char* message, const char* detail) {
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& listStyle = theme.list;

    float padX = Scale(12.0f);
    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0.0f, Scale(4.0f)));

    theme.PushFont(listStyle.subFont ? listStyle.subFont : theme.fontRegular, listStyle.subFontSize);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddText(ImVec2(pos.x + padX, pos.y), listStyle.colTextMuted, message);
    if (detail && detail[0] != '\0') {
        float lineH = ImGui::GetTextLineHeight();
        dl->AddText(ImVec2(pos.x + padX, pos.y + lineH + 2.0f), UiTheme::Get().palette.textMuted, detail);
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, lineH * 2.0f + Scale(8.0f)));
    } else {
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() + Scale(8.0f)));
    }
    theme.PopFont();
}
