#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// Универсальный компонент рабочей области окна (ContentArea). RAII-область:
// конструктор открывает окно, деструктор закрывает и снимает стили ImGui.
// По умолчанию геометрия рассчитывается относительно Header, Toolbar, Sidebar
// и StatusBar:
//
//     if (auto content = ContentArea()) { ... }
// Геометрия области содержимого (итоговые px); значение < 0 берётся по умолчанию:
//
//     ContentArea({.posXPx = 200, .widthPx = 800});
struct ContentAreaOptions {
    float                   posXPx   = -1.0f;
    float                   posYPx   = -1.0f;
    float                   widthPx  = -1.0f;
    float                   heightPx = -1.0f;
    const ContentAreaStyle* style    = nullptr;   // оверрайд стиля; nullptr — из темы
};

class ContentArea : public Scope {
public:
    // Геометрия каркаса: posX = SidebarWidth, posY = HeaderHeight + TopBarHeight,
    // размер до нижнего правого угла (над строкой состояния)
    explicit ContentArea(const ContentAreaOptions& options = {});

    ~ContentArea();

    // Размеры рабочей области
    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    ImVec2 GetAvailableSize() const;

private:
    void Open(float posX, float posY, float width, float height,
              const ContentAreaStyle* customStyle);

    const ContentAreaStyle* m_style = nullptr;
    float m_posX = 0.0f;
    float m_posY = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;
};
