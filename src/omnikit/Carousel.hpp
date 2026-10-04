#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include <imgui.h>

namespace OmniKit {

// Carousel options (designated initializers):
//
//     if (Carousel carousel({.widthPx = zone.Width(), .heightPx = zone.Height()}); carousel) { ... }
struct CarouselOptions {
    float widthPx  = 0.0f;                  // resolved width, px; 0 — fill available space
    float heightPx = 0.0f;                  // resolved height, px; 0 — indicator height from theme
    const char* key = nullptr;              // identity (needed if multiple carousels share an area); nullptr — "##carousel"
    const CarouselStyle* style = nullptr;   // style override; nullptr — from theme
};

// ============================================================================
// Horizontal scroll ribbon with navigation arrows (RAII scope): constructor
// opens the ribbon, destructor closes it and renders the right arrow.
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

} // namespace OmniKit
