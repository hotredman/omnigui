#include "ui/components/Sidebar.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

SidebarWrappedLabel SidebarWrapLabel(const char* text, float maxWidth) {
    SidebarWrappedLabel out;
    const std::string full = text ? text : "";
    const char* begin = full.c_str();
    const char* end = begin + full.size();
    ImFont* font = ImGui::GetFont();
    const float size = ImGui::GetFontSize();

    const char* cut = font->CalcWordWrapPosition(size, begin, end, maxWidth);
    if (cut >= end) {
        out.line1 = full;
        return out;
    }
    if (cut <= begin)   // окно уже слова: хотя бы один символ в строке
        cut = begin + 1;
    while (cut < end && (static_cast<unsigned char>(*cut) & 0xC0) == 0x80)   // не резать UTF-8 посреди символа
        ++cut;

    std::string first(begin, cut);
    while (!first.empty() && first.back() == ' ')
        first.pop_back();
    out.line1 = first;

    while (cut < end && *cut == ' ')
        ++cut;
    std::string rest(cut, end);
    if (rest.empty())
        return out;
    if (ImGui::CalcTextSize(rest.c_str()).x <= maxWidth) {
        out.line2 = rest;
        return out;
    }

    // Остаток длиннее строки: самый длинный префикс, с которым помещается «…»
    const std::string ellipsis = "\xE2\x80\xA6";
    const float room = maxWidth - ImGui::CalcTextSize(ellipsis.c_str()).x;
    size_t keep = 0;
    for (size_t i = 0; i < rest.size();) {
        size_t next = i + 1;
        while (next < rest.size() && (static_cast<unsigned char>(rest[next]) & 0xC0) == 0x80)
            ++next;
        if (ImGui::CalcTextSize(rest.substr(0, next).c_str()).x > room)
            break;
        keep = next;
        i = next;
    }
    std::string head = rest.substr(0, keep);
    while (!head.empty() && head.back() == ' ')
        head.pop_back();
    out.line2 = head + ellipsis;
    return out;
}

Sidebar::Sidebar(const SidebarOptions& options) {
    const float posY = options.posYPx;
    const float height = options.heightPx;
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
    const float indent = theme.Scale(kSidebarTreeIndent) * static_cast<float>(o.level);
    const ImU32 statusColor = o.statusColor;
    if (statusColor != 0) {
        float dotRadius = theme.Scale(3.5f);
        ImVec2 dotCenter(cursor.x + indent + theme.Scale(18.0f), cursor.y + itemHeight * 0.5f);
        dl->AddCircleFilled(dotCenter, dotRadius, statusColor);
    }

    float textX = cursor.x + indent + (statusColor != 0 ? theme.Scale(30.0f) : theme.Scale(12.0f));
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

SidebarTreeClick SidebarList::TreeNode(const SidebarTreeNodeOptions& o) {
    if (!m_open) return SidebarTreeClick::None;

    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = theme.sidebar;
    const ListStyle& listStyle = theme.list;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    const float itemWidth = ImGui::GetContentRegionAvail().x;
    const float cornerRadius = theme.Scale(style.cornerRadius);
    const float indent = theme.Scale(kSidebarTreeIndent) * static_cast<float>(o.level);
    const float arrowZoneRight = cursor.x + indent + theme.Scale(28.0f);   // стрелка раскрытия: клик = Toggle

    // Справа: счётчик и индикатор статуса — их ширина нужна до названия
    float countW = 0.0f;
    ImFont* subFont = listStyle.subFont ? listStyle.subFont : theme.fontRegular;
    if (!o.count.empty()) {
        theme.PushFont(subFont, listStyle.subFontSize);
        countW = ImGui::CalcTextSize(o.count.c_str()).x;
        theme.PopFont();
    }
    float rightEdge = cursor.x + itemWidth - theme.Scale(10.0f);
    const float countRight = rightEdge;
    rightEdge -= countW > 0.0f ? countW + theme.Scale(8.0f) : 0.0f;
    const float dotCenterX = rightEdge - theme.Scale(3.5f);
    if (o.statusColor != 0)
        rightEdge -= theme.Scale(14.0f);

    // Название переносится на вторую строку, вторая при нехватке места оканчивается многоточием
    const float textX = cursor.x + indent + theme.Scale(28.0f);
    const float maxTextW = std::max(10.0f, rightEdge - textX);
    ImFont* font = listStyle.itemFont ? listStyle.itemFont : theme.fontMedium;
    theme.PushFont(font, listStyle.itemFontSize);
    const SidebarWrappedLabel text = SidebarWrapLabel(o.label.c_str(), maxTextW);
    const float lineH = ImGui::GetTextLineHeight();
    const float textH = text.line2.empty() ? lineH : lineH * 2.0f + theme.Scale(2.0f);
    const float itemHeight = std::max(theme.Scale(36.0f), textH + theme.Scale(18.0f));
    const float textY = cursor.y + (itemHeight - textH) * 0.5f;
    const float firstLineMidY = textY + lineH * 0.5f;

    if (o.selected)
        dl->AddRectFilled(cursor, ImVec2(cursor.x + itemWidth, cursor.y + itemHeight), style.colActiveBg, cornerRadius);

    ImGui::PushID(o.key ? o.key : o.label.c_str());
    const bool pressed = ImGui::InvisibleButton("##tree_node", ImVec2(itemWidth, itemHeight));
    ImGui::PopID();
    const bool isHovered = ImGui::IsItemHovered();
    if (isHovered && !o.selected)
        dl->AddRectFilled(cursor, ImVec2(cursor.x + itemWidth, cursor.y + itemHeight), style.colHoverBg, cornerRadius);

    const ImU32 textCol = o.selected ? style.colActiveText : (isHovered ? style.colHoverText : style.colInactiveText);

    // Стрелка раскрытия: вправо — свёрнут, вниз — раскрыт; по первой строке названия
    {
        const float cx = cursor.x + indent + theme.Scale(14.0f);
        const float r = theme.Scale(4.0f);
        if (o.expanded) {
            dl->AddTriangleFilled(ImVec2(cx - r, firstLineMidY - r * 0.55f), ImVec2(cx + r, firstLineMidY - r * 0.55f),
                                  ImVec2(cx, firstLineMidY + r * 0.75f), textCol);
        } else {
            dl->AddTriangleFilled(ImVec2(cx - r * 0.55f, firstLineMidY - r), ImVec2(cx - r * 0.55f, firstLineMidY + r),
                                  ImVec2(cx + r * 0.75f, firstLineMidY), textCol);
        }
    }

    dl->PushClipRect(ImVec2(textX, cursor.y), ImVec2(textX + maxTextW, cursor.y + itemHeight), true);
    dl->AddText(ImVec2(textX, textY), textCol, text.line1.c_str());
    if (!text.line2.empty())
        dl->AddText(ImVec2(textX, textY + lineH + theme.Scale(2.0f)), textCol, text.line2.c_str());
    dl->PopClipRect();
    theme.PopFont();

    // Счётчик и индикатор — на уровне первой строки
    if (countW > 0.0f) {
        theme.PushFont(subFont, listStyle.subFontSize);
        dl->AddText(ImVec2(countRight - countW, firstLineMidY - ImGui::GetTextLineHeight() * 0.5f),
                    style.colInactiveText, o.count.c_str());
        theme.PopFont();
    }
    if (o.statusColor != 0)
        dl->AddCircleFilled(ImVec2(dotCenterX, firstLineMidY), theme.Scale(3.5f), o.statusColor);

    if (!pressed) return SidebarTreeClick::None;
    return ImGui::GetMousePos().x < arrowZoneRight ? SidebarTreeClick::Toggle : SidebarTreeClick::Activate;
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
