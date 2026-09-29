#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <string>

class SidePanel {
public:
    enum class Side {
        Left,
        Right
    };

    // 1. Фиксированная панель (без сплиттера)
    SidePanel(
        const char* id,
        float width,
        Side side = Side::Right,
        const SidePanelStyle& style = UiTheme::Get().sidePanel
    );

    // 2. Интерактивная панель со сплиттером
    SidePanel(
        const char* id,
        float& width,
        Side side,
        bool resizable,
        float minWidth = 240.0f,
        float maxWidth = 600.0f,
        const SidePanelStyle& style = UiTheme::Get().sidePanel
    );

    ~SidePanel();

    // Запрет копирования
    SidePanel(const SidePanel&) = delete;
    SidePanel& operator=(const SidePanel&) = delete;

    // Явное закрытие панели
    void End();

    // Проверка успешности открытия ImGui::BeginChild
    operator bool() const { return m_open; }

    // Текущая базовая ширина (без масштаба)
    float GetWidth() const { return m_width; }

    // Суммарная занимаемая ширина в пикселях экрана с учетом масштаба и сплиттера
    float GetTotalWidth() const;

    // Статический расчет занимаемой ширины (в пикселях экрана с учетом масштаба)
    static float CalcTotalWidth(
        float width,
        bool resizable = false,
        const SidePanelStyle& style = UiTheme::Get().sidePanel
    );

private:
    void Init(const char* id, float width, float* widthRef, Side side, bool resizable,
              float minWidth, float maxWidth, const SidePanelStyle& style);
    void RenderSplitter();

    std::string m_id;
    float m_width = 320.0f;
    float* m_widthRef = nullptr;
    Side m_side = Side::Right;
    bool m_resizable = false;
    float m_minWidth = 240.0f;
    float m_maxWidth = 600.0f;
    SidePanelStyle m_style;

    bool m_open = false;
    bool m_childStarted = false;
    int m_styleColorPushes = 0;
    int m_styleVarPushes = 0;
};
