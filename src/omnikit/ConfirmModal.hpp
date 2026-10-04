#pragma once

#include "omnikit/UiTheme.hpp"

#include <string>

namespace OmniKit {

// Confirmation modal options (designated initializers):
//
//     ConfirmModal deleteRun({.title = "Delete run?", .message = "This cannot be undone.",
//                             .confirmLabel = "Delete"});
struct ConfirmModalOptions {
    const char* title        = "";
    const char* message      = "";
    const char* confirmLabel = "Confirm";
    UiVariant   variant      = UiVariant::Danger;   // confirmation button color variant
};

// ============================================================================
// Confirmation modal dialog: title, descriptive text, Cancel and confirm
// buttons (colored by variant). The object stores its own open/closed state,
// so explicit window identifiers are not needed.
// ============================================================================
class ConfirmModal {
public:
    explicit ConfirmModal(const ConfirmModalOptions& options = {});

    void Open() { m_open = true; }
    // Message dependent on runtime values: set prior to Open()
    void SetMessage(std::string message) { m_message = std::move(message); }
    bool IsOpen() const { return m_open; }

    // Called once per frame; returns true when the operator confirms
    bool Render();

private:
    std::string m_title;
    std::string m_message;
    std::string m_confirmLabel;
    UiVariant m_variant;
    bool m_open = false;
};

} // namespace OmniKit
