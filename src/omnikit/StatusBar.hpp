#pragma once

#include "omnikit/Scope.hpp"
#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <source_location>

namespace OmniKit {

// Status bar at the bottom of the window (StatusBar): full-width bar
// beneath the working area. Content consists of single-line elements left to right:
// text items and vertical separators. The last text item can fill the remaining
// width (maxWidth = 0): long text is truncated with ellipsis, with full
// text displayed in a hover tooltip.
//
// Geometry comes from UiTheme (StatusBarHeight): Sidebar and ContentArea
// end above the status bar by default.
//
// RAII scope: constructor opens the window, destructor closes it.
//
//     if (auto status = StatusBar()) { status.Text("Ready"); }
struct StatusBarOptions {
    const StatusBarStyle* style = nullptr;   // Style override; nullptr uses theme
};

// Status bar text options:
//
//     status.Text("Uplink: active", {.variant = UiVariant::Success});
struct StatusBarTextOptions {
    UiVariant variant  = UiVariant::Default;   // Text color (Default is standard status bar text color)
    float     maxWidth = 0.0f;                 // Base px; 0 fits text width
    UiKey     key      = {};
};

class StatusBar : public Scope {
public:
    explicit StatusBar(const StatusBarOptions& options = {});
    ~StatusBar();

    // Single-line text. variant specifies color (Default is secondary text
    // for the bar); maxWidth = 0 extends to the right edge of the bar
    void Text(const std::string& text, const StatusBarTextOptions& options = {},
              std::source_location loc = std::source_location::current());

    // Vertical separator between elements
    void Separator();

    float GetHeight() const { return m_height; }

private:
    // Advance to next item in line (previous item + spacing)
    void NextItem();

    // Explicitly provided style; nullptr uses current theme style (updates dynamically on theme change)
    const StatusBarStyle* m_customStyle = nullptr;
    const StatusBarStyle& Style() const { return m_customStyle ? *m_customStyle : UiTheme::Get().statusBar; }
    bool m_hasItems = false;
    int m_textCount = 0;   // Sequential text index: unique ID for each Text()
    float m_height = 0.0f;
};

} // namespace OmniKit
