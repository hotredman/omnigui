#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include <imgui.h>

namespace OmniKit {

// Universal main content area component (ContentArea). RAII scope:
// constructor opens the window, destructor closes it and pops ImGui styles.
// By default, geometry is computed relative to Header, Toolbar, Sidebar,
// and StatusBar:
//
//     if (auto content = ContentArea()) { ... }
// Content area geometry (resolved px); values < 0 fall back to defaults:
//
//     ContentArea({.posXPx = 200, .widthPx = 800});
struct ContentAreaOptions {
    float                   posXPx   = -1.0f;
    float                   posYPx   = -1.0f;
    float                   widthPx  = -1.0f;
    float                   heightPx = -1.0f;
    const ContentAreaStyle* style    = nullptr;   // style override; nullptr — from theme
};

class ContentArea : public Scope {
public:
    // Scaffold geometry: posX = SidebarWidth, posY = HeaderHeight + TopBarHeight,
    // sized to the bottom-right corner (above status bar)
    explicit ContentArea(const ContentAreaOptions& options = {});

    ~ContentArea();

    // Content area dimensions
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

} // namespace OmniKit
