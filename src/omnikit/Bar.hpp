#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include "omnikit/Button.hpp"
#include <imgui.h>

namespace OmniKit {

// Common foundation for full-width horizontal window bars (Header, Toolbar):
// a full-width window with three zones — Left, Center, and Right. Left/Right widths
// are measured after rendering and cached in ImGuiStorage, so Center receives
// the remaining space and the bar object itself remains stateless between frames.

// Base (unscaled) bar metrics — common subset of HeaderStyle and ToolbarStyle
struct BarMetrics {
    float height;
    float paddingX;
    float paddingY;
    float zoneSpacing;
    float separatorSize;
    float defaultLeftWidth;
    float defaultRightWidth;
    ImU32 colBg;
    ImU32 colSeparator;
};

template <typename Style>
BarMetrics MakeBarMetrics(const Style& s) {
    return { s.height, s.paddingX, s.paddingY, s.zoneSpacing, s.separatorSize,
             s.defaultLeftWidth, s.defaultRightWidth, s.colBg, s.colSeparator };
}

// Bar geometry in pixels (after scaling), shared across zones
struct BarLayout {
    float paddingX = 0.0f;
    float paddingY = 0.0f;
    float zoneSpacing = 0.0f;
    float contentHeight = 0.0f;      // zone client area height, px
    float baseContentHeight = 0.0f;  // client area height in base units (for components that scale internally)
    float leftWidth = 0.0f;
    float rightWidth = 0.0f;
    ImGuiID storageId = 0;
};

class Bar;

// Bar zone. Not instantiated directly: use Bar::Left()/Center()/Right()
class BarZone : public Scope {
public:
    // Height of the zone client area, px
    float Height() const { return m_layout ? m_layout->contentHeight : 0.0f; }

    // Label vertically centered within the bar
    void Label(const char* text, UiVariant variant = UiVariant::Secondary);

    // Button sized to fill the zone content height (options.size is ignored)
    bool Button(const ButtonOptions& options);

protected:
    BarZone(BarLayout* layout, bool active);
    ~BarZone() = default;

    // Closes the zone group and returns measured width
    float EndZone();

    BarLayout* m_layout = nullptr;
};

class LeftZone : public BarZone {
public:
    ~LeftZone();
private:
    friend class Bar;
    LeftZone(BarLayout* layout, bool active);
};

class CenterZone : public BarZone {
public:
    ~CenterZone();
    // Remaining width between left and right zones, px
    float Width() const { return m_width; }
private:
    friend class Bar;
    CenterZone(BarLayout* layout, bool active);
    float m_width = 0.0f;
};

class RightZone : public BarZone {
public:
    ~RightZone();
private:
    friend class Bar;
    RightZone(BarLayout* layout, bool active);
};

class Bar : public Scope {
public:
    ~Bar();

    LeftZone Left();
    CenterZone Center();
    RightZone Right();

    float GetHeight() const { return m_height; }
    float GetContentHeight() const { return m_layout.contentHeight; }

protected:
    // windowName / zonesId — window identifier and zone width storage identifier;
    // posY — bar top coordinate (px), height <= 0 — height from metrics
    Bar(const BarMetrics& metrics, const char* windowName, const char* zonesId,
        float posY, float height);

private:
    BarLayout m_layout;
    float m_height = 0.0f;
};

} // namespace OmniKit
