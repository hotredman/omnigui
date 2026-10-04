#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include <imgui.h>
#include <string>

namespace OmniKit {

enum class SidePanelSide {
    Left,
    Right
};

// Side panel options (designated initializers):
//
//     if (SidePanel panel({.width = 260}); panel) { ... }                         // fixed width
//     if (SidePanel panel(s_width, {.minWidth = 240, .maxWidth = 600}); panel) { ... } // with splitter
struct SidePanelOptions {
    float                width    = 320.0f;                 // Base px; for panel with splitter, sourced from passed variable
    SidePanelSide        side     = SidePanelSide::Right;
    float                minWidth = 240.0f;                 // Width bounds for panel with splitter
    float                maxWidth = 600.0f;
    const char*          key      = nullptr;                // Identity; nullptr uses "##SidePanel" (needed if multiple panels share a region)
    const SidePanelStyle* style   = nullptr;                // Style override; nullptr uses theme
};

// ============================================================================
// Side panel (RAII scope): child window of specified width filling all
// remaining height. With a splitter, width can be resized with mouse drag.
// ============================================================================
class SidePanel : public Scope {
public:
    using Side = SidePanelSide;

    // 1. Fixed width (without splitter)
    explicit SidePanel(const SidePanelOptions& options = {});

    // 2. Resizable width with splitter: width is read and written by the panel
    SidePanel(float& width, const SidePanelOptions& options = {});

    ~SidePanel();

    // Current width (base px)
    float GetWidth() const { return m_width; }

    // Total width including splitter in px (scaled)
    float GetTotalWidth() const;

    // Static calculation of total width (in pixels, scaled)
    static float CalcTotalWidth(
        float width,
        bool resizable = false,
        const SidePanelStyle* style = nullptr   // nullptr uses theme style
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

} // namespace OmniKit
