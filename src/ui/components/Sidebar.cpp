#include "ui/components/Sidebar.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

Sidebar::Sidebar(float posY, float height) {
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = theme.sidebar;

    const float posYPx = (posY >= 0.0f) ? posY : (theme.HeaderHeight() + theme.TopBarHeight());
    const float width = theme.SidebarWidth();
    // По умолчанию — до строки состояния внизу окна
    const float heightPx = (height >= 0.0f)
        ? height
        : (ImGui::GetIO().DisplaySize.y - posYPx - UiTheme::Get().StatusBarHeight());

    ImGui::SetNextWindowPos(ImVec2(0, posYPx));
    ImGui::SetNextWindowSize(ImVec2(width, heightPx));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Scale(6.0f), Scale(10.0f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, Scale(style.itemSpacing)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(style.colBg).Value);

    m_open = ImGui::Begin("##SidebarWindow", nullptr, flags);
    if (m_open) {
        // Правая вертикальная разделительная линия
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddLine(
            ImVec2(width - 1.0f, posYPx),
            ImVec2(width - 1.0f, posYPx + heightPx),
            style.colSeparator,
            1.0f
        );
    }
}

Sidebar::~Sidebar() {
    // ImGui::End() обязателен независимо от результата Begin
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

bool Sidebar::Item(const SidebarItemOptions& o) {
    if (!m_open) return false;

    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = theme.sidebar;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float itemHeight = Scale(style.itemHeight);
    float itemWidth = ImGui::GetContentRegionAvail().x;
    float cornerRadius = Scale(style.cornerRadius);
    const bool isSelected = o.selected;

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

    // 2. Обработка клика и ховера (идентичность — key либо подпись)
    ImGui::PushID(o.key ? o.key : o.label.c_str());
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
    if (o.icon != Icon::None) {
        o.icon.Draw(dl, iconCenter, iconSize, textColor);
    }

    // 5. Отрисовка подписи с клиппированием
    float textX = cursor.x + Scale(38.0f);
    float maxTextW = itemWidth - Scale(44.0f) - (isSelected ? Scale(style.activeBarWidth + 4.0f) : 0.0f);
    if (maxTextW < 10.0f) maxTextW = 10.0f;

    dl->PushClipRect(ImVec2(textX, cursor.y), ImVec2(textX + maxTextW, cursor.y + itemHeight), true);
    dl->AddText(font, fontSize, ImVec2(textX, textY), textColor, o.label.c_str());
    dl->PopClipRect();

    return pressed;
}

void Sidebar::Separator() {
    if (!m_open) return;
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = theme.sidebar;

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float itemWidth = ImGui::GetContentRegionAvail().x;
    float y = cursor.y + Scale(6.0f);
    float x1 = cursor.x + Scale(6.0f);
    float x2 = cursor.x + itemWidth - Scale(6.0f);

    dl->AddLine(ImVec2(x1, y), ImVec2(x2, y), style.colSeparator, 1.0f);
    ImGui::Dummy(ImVec2(0, Scale(12.0f)));
}

void Sidebar::SectionTitle(const std::string& title) {
    if (!m_open) return;
    const UiTheme& theme = UiTheme::Get();
    const ListStyle& listStyle = theme.list;

    ImFont* font = listStyle.headerFont ? listStyle.headerFont : theme.fontBold;
    theme.PushFont(font, listStyle.headerFontSize);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float padX = Scale(10.0f);
    float lineHeight = ImGui::GetTextLineHeight();
    float headerH = lineHeight + Scale(6.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddText(ImVec2(pos.x + padX, pos.y + Scale(2.0f)), listStyle.colHeader, title.c_str());

    theme.PopFont();
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + headerH));
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
}

void Sidebar::Spacer(float basePx) {
    if (!m_open) return;
    ImGui::Dummy(ImVec2(0, Scale(basePx)));
}

SidebarList Sidebar::ScrollList() {
    return SidebarList();
}

// ----------------------------------------------------------------------------
// SidebarList: прокручиваемая область под список записей
// ----------------------------------------------------------------------------
SidebarList::SidebarList() {
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = theme.sidebar;

    float availH = ImGui::GetContentRegionAvail().y;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, theme.Scale(style.itemSpacing)));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, style.colScrollRegionBg);  // подложка — только окну блока
    m_open = ImGui::BeginChild("##SidebarScrollList", ImVec2(0.0f, availH), false, ImGuiWindowFlags_NoSavedSettings);
    ImGui::PopStyleColor();
}

SidebarList::~SidebarList() {
    // EndChild обязателен, даже если BeginChild вернул false
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

bool SidebarList::Entry(const SidebarEntryOptions& o) {
    if (!m_open) return false;

    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = theme.sidebar;
    const ListStyle& listStyle = theme.list;
    const bool isSelected = o.selected;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float itemHeight = theme.Scale(48.0f);
    float itemWidth = ImGui::GetContentRegionAvail().x;
    float cornerRadius = theme.Scale(style.cornerRadius);

    // 1. Активный фон и правый индикатор
    if (isSelected) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colActiveBg,
            cornerRadius
        );

        float barWidth = theme.Scale(style.activeBarWidth);
        float barPad = theme.Scale(4.0f);
        dl->AddRectFilled(
            ImVec2(cursor.x + itemWidth - barWidth, cursor.y + barPad),
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight - barPad),
            style.colActiveBar,
            barWidth * 0.5f
        );
    }

    // 2. Обработка клика и ховера (идентичность — key либо название)
    ImGui::PushID(o.key ? o.key : o.label.c_str());
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

    // 3. Опциональный статусный светодиод (LED dot)
    const ImU32 statusColor = o.statusColor;
    if (statusColor != 0) {
        float dotRadius = theme.Scale(3.5f);
        ImVec2 dotCenter(cursor.x + theme.Scale(18.0f), cursor.y + itemHeight * 0.5f);
        dl->AddCircleFilled(dotCenter, dotRadius, statusColor);
    }

    float textX = cursor.x + (statusColor != 0 ? theme.Scale(30.0f) : theme.Scale(12.0f));
    float maxTextW = itemWidth - (textX - cursor.x) - (isSelected ? theme.Scale(style.activeBarWidth + 6.0f) : theme.Scale(4.0f));
    if (maxTextW < 10.0f) maxTextW = 10.0f;

    const bool hasSub = !o.sublabel.empty();
    float textH = hasSub ? (theme.Scale(listStyle.itemFontSize) + theme.Scale(listStyle.subFontSize) + theme.Scale(2.0f))
                         : theme.Scale(listStyle.itemFontSize);
    float line1Y = cursor.y + (itemHeight - textH) * 0.5f;

    // 4. Первая строка: название команды
    ImFont* font = listStyle.itemFont ? listStyle.itemFont : theme.fontMedium;
    theme.PushFont(font, listStyle.itemFontSize);
    ImU32 textCol = isSelected ? style.colActiveText : (isHovered ? style.colHoverText : style.colInactiveText);

    dl->PushClipRect(ImVec2(textX, cursor.y), ImVec2(textX + maxTextW, cursor.y + itemHeight), true);
    dl->AddText(ImVec2(textX, line1Y), textCol, o.label.c_str());
    dl->PopClipRect();
    theme.PopFont();

    // 5. Вторая строка: дата и время
    if (hasSub) {
        ImFont* subFont = listStyle.subFont ? listStyle.subFont : theme.fontRegular;
        theme.PushFont(subFont, listStyle.subFontSize);
        float line2Y = line1Y + theme.Scale(listStyle.itemFontSize) + theme.Scale(2.0f);
        ImU32 subCol = isSelected ? theme.palette.textSecondary : style.colInactiveText;

        dl->PushClipRect(ImVec2(textX, cursor.y), ImVec2(textX + maxTextW, cursor.y + itemHeight), true);
        dl->AddText(ImVec2(textX, line2Y), subCol, o.sublabel.c_str());
        dl->PopClipRect();
        theme.PopFont();
    }

    return pressed;
}

void SidebarList::Empty(const std::string& message, const std::string& detail) {
    if (!m_open) return;

    const UiTheme& theme = UiTheme::Get();
    const ListStyle& listStyle = theme.list;

    float padX = theme.Scale(12.0f);
    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0.0f, theme.Scale(4.0f)));

    theme.PushFont(listStyle.subFont ? listStyle.subFont : theme.fontRegular, listStyle.subFontSize);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddText(ImVec2(pos.x + padX, pos.y), listStyle.colTextMuted, message.c_str());
    if (!detail.empty()) {
        float lineH = ImGui::GetTextLineHeight();
        dl->AddText(ImVec2(pos.x + padX, pos.y + lineH + 2.0f), theme.palette.textMuted, detail.c_str());
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, lineH * 2.0f + theme.Scale(8.0f)));
    } else {
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() + theme.Scale(8.0f)));
    }
    theme.PopFont();
}
