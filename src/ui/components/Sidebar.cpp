#include "ui/components/Sidebar.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cmath>

Sidebar::Sidebar() {
}

Sidebar::~Sidebar() {
    if (m_beginCalled) {
        End();
    }
}

bool Sidebar::Begin(float posY, float height) {
    const UiTheme& theme = UiTheme::Get();
    const SidebarStyle& style = theme.sidebar;

    m_posY = (posY >= 0.0f) ? posY : (theme.HeaderHeight() + theme.TopBarHeight());
    m_width = theme.SidebarWidth();
    // По умолчанию — до строки состояния внизу окна
    m_height = (height >= 0.0f)
        ? height
        : (ImGui::GetIO().DisplaySize.y - m_posY - UiTheme::Get().StatusBarHeight());

    ImGui::SetNextWindowPos(ImVec2(0, m_posY));
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Scale(6.0f), Scale(10.0f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, Scale(style.itemSpacing)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(style.colBg).Value);

    m_beginCalled = true;
    m_open = ImGui::Begin("##SidebarWindow", nullptr, flags);
    if (m_open) {
        // Правая вертикальная разделительная линия
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddLine(
            ImVec2(m_width - 1.0f, m_posY),
            ImVec2(m_width - 1.0f, m_posY + m_height),
            style.colSeparator,
            1.0f
        );
    }
    return m_open;
}

void Sidebar::End() {
    if (m_beginCalled) {
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
        m_beginCalled = false;
        m_open = false;
    }
}

bool Sidebar::AddItem(const char* id, const char* label, Icon icon, bool isActive) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    const SidebarStyle& style = UiTheme::Get().sidebar;
    float itemHeight = Scale(style.itemHeight);
    float itemWidth = m_width - Scale(12.0f);
    float cornerRadius = Scale(style.cornerRadius);

    // 1. Активный фон и правый неоново-голубой индикатор
    if (isActive) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colActiveBg,
            cornerRadius
        );

        // Правый вертикальный индикатор
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
    bool clicked = ImGui::InvisibleButton("##nav_item", ImVec2(itemWidth, itemHeight));
    ImGui::PopID();
    bool isHovered = ImGui::IsItemHovered();

    if (isHovered && !isActive) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colHoverBg,
            cornerRadius
        );
    }

    // 3. Цвета и шрифты
    ImU32 textColor = isActive ? style.colActiveText
                               : (isHovered ? style.colHoverText : style.colInactiveText);

    ImFont* font = isActive ? (style.activeFont ? style.activeFont : style.font) : style.font;
    if (!font) font = UiTheme::Get().defaultFont;
    float fontSize = Scale(style.fontSize);
    float textY = cursor.y + (itemHeight - fontSize) * 0.5f - 1.0f;

    // 4. Отрисовка векторной иконки
    float iconSize = Scale(style.iconSize);
    ImVec2 iconCenter(cursor.x + Scale(24.0f), cursor.y + itemHeight * 0.5f);
    DrawVectorIcon(icon, dl, iconCenter, iconSize, textColor);

    // 5. Отрисовка подписи
    dl->AddText(font, fontSize, ImVec2(cursor.x + Scale(44.0f), textY), textColor, label);

    return clicked;
}

bool Sidebar::AddItem(const char* id, const char* label, const char* iconStr, bool isActive) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    const SidebarStyle& style = UiTheme::Get().sidebar;
    float itemHeight = Scale(style.itemHeight);
    float itemWidth = m_width - Scale(12.0f);
    float cornerRadius = Scale(style.cornerRadius);

    if (isActive) {
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

    ImGui::PushID(id);
    bool clicked = ImGui::InvisibleButton("##nav_item", ImVec2(itemWidth, itemHeight));
    ImGui::PopID();
    bool isHovered = ImGui::IsItemHovered();

    if (isHovered && !isActive) {
        dl->AddRectFilled(
            cursor,
            ImVec2(cursor.x + itemWidth, cursor.y + itemHeight),
            style.colHoverBg,
            cornerRadius
        );
    }

    ImU32 textColor = isActive ? style.colActiveText
                               : (isHovered ? style.colHoverText : style.colInactiveText);

    ImFont* font = isActive ? (style.activeFont ? style.activeFont : style.font) : style.font;
    if (!font) font = UiTheme::Get().defaultFont;
    float fontSize = Scale(style.fontSize);
    float textY = cursor.y + (itemHeight - fontSize) * 0.5f - 1.0f;

    if (iconStr && iconStr[0] != '\0') {
        dl->AddText(font, fontSize, ImVec2(cursor.x + Scale(14.0f), textY), textColor, iconStr);
    }

    float textX = (iconStr && iconStr[0] != '\0') ? (cursor.x + Scale(44.0f)) : (cursor.x + Scale(18.0f));
    dl->AddText(font, fontSize, ImVec2(textX, textY), textColor, label);

    return clicked;
}

void Sidebar::AddSeparator() {
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float y = cursor.y + Scale(6.0f);
    float x1 = cursor.x + Scale(8.0f);
    float x2 = cursor.x + m_width - Scale(20.0f);

    dl->AddLine(ImVec2(x1, y), ImVec2(x2, y), UiTheme::Get().sidebar.colSeparator, 1.0f);
    ImGui::Dummy(ImVec2(0, Scale(12.0f)));
}

void Sidebar::AddSpacing(float height) {
    ImGui::Dummy(ImVec2(0, Scale(height)));
}

void Sidebar::DrawVectorIcon(Icon icon, ImDrawList* dl, ImVec2 center, float size, ImU32 color) {
    icon.Draw(dl, center, size, color);
}
