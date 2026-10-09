#pragma once

#include <source_location>
#include <string_view>
#include "kvant/core/Response.hpp"
#include "kvant/core/Tokens.hpp"
#include "kvant/core/Types.hpp"
#include "kvant/graphics/Icon.hpp"

namespace kvant {

class Context;
class ICanvas;

/// Clean configuration options for a Button widget.
struct ButtonOptions {
    std::string_view label{};
    Icon icon{};
    UiVariant variant{UiVariant::Secondary};
    Placement iconPlacement{Placement::Leading};
    bool disabled{false};
};

/// Type alias for button response.
using ButtonResponse = Response;

/// Autonomous Button widget logic (execution, measuring, and rendering).
class Button {
public:
    /// Primary immediate mode entry point for Button widget.
    static Response Run(
        Context& ctx,
        const ButtonOptions& opts,
        std::source_location loc = std::source_location::current()
    );

    /// Measures intrinsic dimensions based on content (icon + label + padding for current UiSize).
    static Vec2 Measure(const ButtonOptions& opts, UiSize size, ICanvas* canvas = nullptr);

    /// Renders the button into its allocated bounds according to interaction and disabled state.
    static void Render(
        ICanvas& canvas,
        Rect bounds,
        const ButtonOptions& opts,
        UiSize size,
        bool hovered,
        bool pressed,
        bool disabled
    );
};

} // namespace kvant
