#pragma once

#include "omnikit/Bar.hpp"

namespace OmniKit {

// Toolbar beneath Header: Left / Center(Fill) / Right zones.
// RAII scope: constructor opens window, destructor closes it.
//
//     if (auto toolbar = Toolbar()) {
//         if (auto left = toolbar.Left()) { left.Label("Session:"); }
//     }
// Toolbar parameters (designated initializers):
//
//     Toolbar({.heightPx = 48});
struct ToolbarOptions {
    float               posYPx   = -1.0f;    // final px; < 0 — directly under Header
    float               heightPx = 0.0f;     // final px; <= 0 — height from theme style
    const ToolbarStyle* style    = nullptr;  // style override; nullptr — from theme
};

class Toolbar : public Bar {
public:
    explicit Toolbar(const ToolbarOptions& options = {});

    // Central zone occupying remaining width (synonym for Center)
    CenterZone Fill() { return Center(); }
};

} // namespace OmniKit
