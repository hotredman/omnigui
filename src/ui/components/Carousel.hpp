#pragma once

#include "ui/components/Scope.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>

// Параметры карусели (designated initializers):
//
//     if (Carousel carousel({.widthPx = zone.Width(), .heightPx = zone.Height()}); carousel) { ... }
struct CarouselOptions {
    float widthPx  = 0.0f;                  // итоговая ширина, px; 0 — всё свободное место
    float heightPx = 0.0f;                  // итоговая высота, px; 0 — высота индикатора темы
    const char* key = nullptr;              // идентичность (нужна, если каруселей несколько в одной области); nullptr — "##carousel"
    const CarouselStyle* style = nullptr;   // оверрайд стиля; nullptr — из темы
};

// ============================================================================
// Горизонтальная лента с прокруткой и стрелками (RAII-область): конструктор
// открывает ленту, деструктор закрывает и дорисовывает правую стрелку.
// ============================================================================
class Carousel : public Scope {
public:
    explicit Carousel(const CarouselOptions& options = {});
    ~Carousel();

    void EnsureVisible(float itemLeft, float itemRight);
    bool CanScroll() const { return m_canScroll; }
    float GetScrollX() const { return m_scrollX; }
    float GetMaxScrollX() const { return m_maxScrollX; }

private:
    void Begin();
    void End();

    const char* m_id = "##carousel";
    ImVec2 m_requestedSize = ImVec2(0.0f, 0.0f);
    CarouselStyle m_style;
    bool m_ended = false;
    bool m_groupStarted = false;
    bool m_childStarted = false;

    bool m_canScroll = false;
    bool m_showLeftArrow = false;
    bool m_showRightArrow = false;
    float m_arrowW = 0.0f;
    float m_cardH = 0.0f;
    float m_spacing = 0.0f;
    float m_scrollAreaW = 0.0f;
    float m_maxScrollX = 0.0f;
    float m_scrollX = 0.0f;
    float m_targetScrollX = 0.0f;
    ImGuiID m_storageId = 0;
};
