#pragma once

#include "omnikit/Scope.hpp"
#include <imgui.h>

namespace OmniKit {

// ============================================================================
// Disabled interface scope (analogous to <fieldset disabled>): everything
// rendered inside is non-interactive and dimmed (UiTheme::kDisabledAlpha).
// Operates at any nesting level — page, card, field. Scopes nest properly:
// within a disabled scope everything remains disabled, and DisabledScope(false) is a no-op.
//
// An item active at the moment of disabling (e.g., text field with keyboard
// focus) is released so editing does not persist inside a disabled scope.
// ImGui itself only releases hovered mouse items.
// ============================================================================
class DisabledScope : public Scope {
public:
    explicit DisabledScope(bool disabled);
    ~DisabledScope();

    // Whether current render position is within a disabled scope (at any nesting level)
    static bool Active();

private:
    ImGuiID m_activeAliveBefore = 0;  // Active item alive before entering the scope
};

} // namespace OmniKit
