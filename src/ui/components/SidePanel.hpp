#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <string>

enum class SidePanelSide {
    Left,
    Right
};

// Параметры боковой панели (designated initializers):
//
//     if (SidePanel panel({.width = 260}); panel) { ... }                         // фиксированная ширина
//     if (SidePanel panel(s_width, {.minWidth = 240, .maxWidth = 600}); panel) { ... } // со сплиттером
struct SidePanelOptions {
    float                width    = 320.0f;                 // базовые px; для панели со сплиттером берётся из переданной переменной
    SidePanelSide        side     = SidePanelSide::Right;
    float                minWidth = 240.0f;                 // границы ширины для панели со сплиттером
    float                maxWidth = 600.0f;
    const char*          key      = nullptr;                // идентичность; nullptr — "##SidePanel" (нужен, если панелей несколько в одной области)
    const SidePanelStyle* style   = nullptr;                // оверрайд стиля; nullptr — из темы
};

// ============================================================================
// Боковая панель (RAII-область): дочернее окно заданной ширины во всю
// оставшуюся высоту. Со сплиттером ширину можно менять мышью.
// ============================================================================
class SidePanel : public Scope {
public:
    using Side = SidePanelSide;

    // 1. Фиксированная ширина (без сплиттера)
    explicit SidePanel(const SidePanelOptions& options = {});

    // 2. Изменяемая ширина со сплиттером: width читается и пишется панелью
    SidePanel(float& width, const SidePanelOptions& options = {});

    ~SidePanel();

    // Текущая ширина (базовые px)
    float GetWidth() const { return m_width; }

    // Полная ширина вместе со сплиттером в px (с учётом масштаба)
    float GetTotalWidth() const;

    // Статический расчёт полной ширины (в пикселях, с учётом масштаба)
    static float CalcTotalWidth(
        float width,
        bool resizable = false,
        const SidePanelStyle* style = nullptr   // nullptr — стиль из темы
    );

private:
    void Init(const SidePanelOptions& options, float width, float* widthRef, bool resizable);
    void End();
    void RenderSplitter();

    std::string m_id;
    float m_width = 320.0f;
    float* m_widthRef = nullptr;
    Side m_side = Side::Right;
    bool m_resizable = false;
    float m_minWidth = 240.0f;
    float m_maxWidth = 600.0f;
    SidePanelStyle m_style;

    bool m_childStarted = false;
    int m_styleColorPushes = 0;
    int m_styleVarPushes = 0;
};
