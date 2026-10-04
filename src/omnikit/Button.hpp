#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include "omnikit/UiKey.hpp"
#include <imgui.h>
#include <source_location>

namespace OmniKit {

// Button options. Aggregate struct: fields are specified by name (designated initializers),
// order matching declaration; omitted fields take default values.
// Button can be icon-only (without label):
//
//     if (Button({.label = "Save", .variant = UiVariant::Primary, .icon = Icon::Plus})) { ... }
//     Button({.icon = Icon::Cog, .tooltip = "Settings"});
//     Button("OK");   // short form: label only
//
// Button identity (ImGui ID) is automatically derived from the call site (source_location)
// and label/icon; an explicit key (string, int, or pointer) can be provided when needed.
struct ButtonOptions {
    const char* label    = nullptr;           // label text; nullptr — icon-only button
    UiVariant   variant  = UiVariant::Default;
    Icon        icon     = Icon::None;
    UiSize      size     = UiSize::Medium;    // button height from the design system scale
    float       width    = 0.0f;              // base px (unscaled); 0 — fit content; Fill — stretch full width
    const char* tooltip  = nullptr;           // hover tooltip
    bool        disabled = false;
    UiKey       key      = {};                // explicit identity: string, int, or ptr
};

// Design system button. Returns true on the frame it is clicked.
bool Button(const ButtonOptions& options = {},
            std::source_location loc = std::source_location::current());
bool Button(const char* label,
            std::source_location loc = std::source_location::current());

// Natural button width fitting content (label, icon, theme paddings), px
float ButtonWidth(const ButtonOptions& options);

// For container components (Card, panels) that have already computed geometry:
// size in resolved pixels (including scale); an axis set to zero is taken
// from options (width — width, height — size).
// Applications should typically use Button / ButtonWidth.
bool ButtonPx(ImVec2 sizePx, const ButtonOptions& options = {},
              std::source_location loc = std::source_location::current());

// Content-based width for a button with a given height in resolved px (0 — Medium height)
float ButtonWidthPx(const ButtonOptions& options, float heightPx);

} // namespace OmniKit
