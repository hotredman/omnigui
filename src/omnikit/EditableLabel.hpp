#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include "omnikit/Text.hpp"
#include <imgui.h>
#include <string>
#include <functional>

namespace OmniKit {

// Editable label options (designated initializers):
//
//     title.Render(value, {.widthPx = zone.Width(), .defaultValue = [] { return std::string("Session"); }});
struct EditableLabelOptions {
    float                        widthPx      = 0.0f;              // resolved width, px; 0 — fit content, < 0 — fill available space
    TextRole                     role         = TextRole::Title;   // Title — bold title font, Body — regular text
    std::function<std::string()> defaultValue;                     // if set, button in edit mode fills default name
    bool                         editable     = true;              // false — static title without pencil icon or click response
};

class EditableLabel {
public:
    EditableLabel(const EditableLabelStyle* customStyle = nullptr);

    // Renders the editable label.
    // Returns true if the value was modified and committed (Enter, button click, or focus loss).
    // If options.defaultValue is set, clicking the pencil button during edit mode supplies the default name.
    // If options.editable == false, renders as a static label without a pencil icon or click interaction.
    // Identity: the object itself (the label must persist across frames).
    bool Render(std::string& value, const EditableLabelOptions& options = {});

    bool IsEditing() const { return m_editing; }
    void StartEdit(const std::string& currentValue);
    void CancelEdit();
    void CommitEdit(std::string& targetValue);

private:
    // Explicitly passed style; nullptr — current theme style (theme changes take effect immediately)
    const EditableLabelStyle* m_customStyle = nullptr;
    const EditableLabelStyle& Style() const { return m_customStyle ? *m_customStyle : UiTheme::Get().editableLabel; }
    bool m_editing = false;
    char m_buffer[256] = {};
    std::string m_originalValue;
    bool m_focusRequested = false;
    bool m_justOpened = false;
};

} // namespace OmniKit
