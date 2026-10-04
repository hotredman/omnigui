#pragma once

#include "omnikit/UiTheme.hpp"
#include <optional>
#include <string>
#include <vector>

namespace OmniKit {

// Tag chip parameters (designated initializers):
//
//     if (Tag("vibration")) { ... }                       // clickable "# vibration"
//     Tag("readonly", {.clickable = false});
struct TagOptions {
    const char* prefix    = "#";       // prefix before text; nullptr or "" — no prefix
    bool        clickable = true;
    UiSize      size      = UiSize::Medium;  // chip scale by UiSize (Medium is theme style)
    const char* tooltip   = nullptr;   // tooltip; nullptr — "Filter by tag #<text>"
};

// Interactive or informational tag chip in evo-machine-cs style (# tag)
// Returns true if the tag was clicked by the user
bool Tag(const char* text, const TagOptions& options = {});

struct TagListOptions {
    UiSize size      = UiSize::Medium;   // size of all tags in list
    float maxWidthPx = 0.0f;   // wrap width in final px; 0 — all available width
};

// Set of tags with auto-wrapping across available width.
// Returns clicked tag (or std::nullopt if none clicked)
std::optional<std::string> TagList(const std::vector<std::string>& tags,
                                   const TagListOptions& options = {});

} // namespace OmniKit
