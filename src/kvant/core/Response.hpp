#pragma once

#include "Id.hpp"
#include "Types.hpp"

namespace kvant {

class Context;

/// Represents interaction feedback and computed layout bounds for a widget.
struct WidgetResponse {
    Id id{Id::Null()};
    Rect rect{};

    // Mouse and interaction states
    bool clicked{false};
    bool rightClicked{false};
    bool doubleClicked{false};
    bool hovered{false};
    bool pressed{false};      ///< Active/held down during this frame
    bool released{false};     ///< Released inside the widget bounds
    bool focused{false};      ///< Has keyboard/input focus
    bool changed{false};      ///< Value changed (inputs, sliders, toggles)

    Context* ctx{nullptr};    ///< Weak pointer to context for imperative actions

    /// Implicit conversion to bool for idiomatic `if (ui.Button(...))` usage.
    [[nodiscard]] constexpr operator bool() const noexcept {
        return clicked;
    }

    /// Imperatively requests keyboard focus for this widget.
    void Focus();

    /// Imperatively requests the enclosing ScrollArea to bring this widget into visible view.
    void ScrollTo();
};

} // namespace kvant
