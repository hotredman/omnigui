#pragma once

#include <string_view>
#include "kvant/core/Tokens.hpp"
#include "kvant/core/Types.hpp"
#include "kvant/graphics/Icon.hpp"

namespace kvant {

class ICanvas;

/// Placement of icon relative to text inside button.
enum class IconPlacement : uint8_t {
    Leading,  ///< Icon placed to the left of the label
    Trailing  ///< Icon placed to the right of the label
};

/// Clean configuration options for a Button widget.
struct ButtonOptions {
    std::string_view label{};
    Icon icon{};
    UiVariant variant{UiVariant::Secondary};
    IconPlacement iconPlacement{IconPlacement::Leading};
    bool disabled{false};
};

/// Autonomous Button widget logic (measuring and rendering).
class Button {
public:
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
