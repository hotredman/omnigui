#pragma once

#include <imgui.h>
#include "omnikit/UiTheme.hpp"

#include <string>
#include <vector>

namespace OmniKit {

// ============================================================================
// Autocomplete suggestions below a text field: list of values beneath the field frame while
// active. Selection via mouse or arrow keys and Enter; selected value replaces
// field content completely. List is built by caller; the widget only displays it without
// knowing value sources.
//
//     TextSuggest suggest{.entries = {{"Alpha", "12 runs"}, {"Alpha-2", "3 runs"}},
//                         .headline = "New project will be created"};
//     InputField(name, {.label = "Project", .suggest = &suggest});
// ============================================================================
struct TextSuggestEntry {
    std::string value;   // substituted into field
    std::string note;    // muted note on the right ("12 runs · 25.09.2026")
};

struct TextSuggest {
    std::vector<TextSuggestEntry> entries;   // best matches first
    std::string headline;                    // header string above list; empty — none

    bool Empty() const { return entries.empty() && headline.empty(); }
};

// Frame input field state (retrieved right after ImGui::InputText)
struct TextSuggestFieldState {
    bool active = false;        // field is active (input in progress)
    bool activated = false;     // became active in this frame
    bool deactivated = false;   // lost active status in this frame (Enter, click outside)
    bool edited = false;        // text edited in this frame
};

// Draws suggestions popup under freshly rendered field (rectMin/rectMax — field rect; size —
// field size: affects fonts and row height). Called from InputField;
// direct call only needed when wrapping ImGui::InputText manually.
// Returns true if value was selected: value is already replaced. Call immediately after field,
// before rendering the next widget.
bool DrawTextSuggest(ImVec2 rectMin, ImVec2 rectMax, UiSize size, const TextSuggestFieldState& field,
                     const TextSuggest& suggest, std::string& value);

} // namespace OmniKit
