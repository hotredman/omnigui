#pragma once

#include "omnikit/Bar.hpp"

namespace OmniKit {

// Full-width top window bar (Header): Left / Center / Right zones.
// RAII scope: constructor opens window, destructor closes it.
//
//     if (auto header = Header()) {
//         if (auto left = header.Left()) { ... }
//     }
// Header options (designated initializers):
//
//     Header({.heightPx = 64});
struct HeaderOptions {
    float              heightPx = 0.0f;      // resolved px; <= 0 — height from theme style
    const HeaderStyle* style    = nullptr;   // style override; nullptr — from theme
};

class Header : public Bar {
public:
    explicit Header(const HeaderOptions& options = {});
};

} // namespace OmniKit
