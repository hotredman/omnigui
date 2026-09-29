#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// Универсальный компонент рабочей области окна (ContentArea)
// Автоматически рассчитывает геометрию относительно Header, ProjectBar и Sidebar.
// Гарантирует сбалансированность жизненного цикла Begin/End и снятие стилей ImGui.
class ContentArea {
public:
    ContentArea(const ContentAreaStyle* customStyle = nullptr);
    ~ContentArea();

    ContentArea(const ContentArea&) = delete;
    ContentArea& operator=(const ContentArea&) = delete;

    // Открывает окно рабочей зоны с автоматическим вычислением геометрии каркаса
    // (posX = SidebarWidth, posY = HeaderHeight + TopBarHeight, размер до нижнего правого угла)
    bool Begin(float customPosX = -1.0f,
               float customPosY = -1.0f,
               float customWidth = -1.0f,
               float customHeight = -1.0f,
               const ContentAreaStyle* customStyle = nullptr);

    // Закрывает окно рабочей зоны и восстанавливает стек стилей ImGui
    void End();

    // Размеры рабочей области
    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    ImVec2 GetAvailableSize() const;

private:
    const ContentAreaStyle* m_style = nullptr;
    float m_posX = 0.0f;
    float m_posY = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;

    bool m_open = false;
    bool m_beginCalled = false;
};
