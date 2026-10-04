#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/Icon.hpp"
#include <imgui.h>
#include <optional>
#include <string>

namespace OmniKit {

// Text role: determines font and default color
enum class TextRole {
    Body,   // standard body text
    Title,  // title heading (bold)
    Muted   // muted secondary text
};

// Text parameters (designated initializers):
//
//     Text("Connected", {.variant = UiVariant::Success, .icon = Icon::Check});
//     Text("Section", {.role = TextRole::Title});
//     Text(longDescription, {.variant = UiVariant::Secondary, .wrap = true});
struct TextOptions {
    UiVariant             variant = UiVariant::Default;   // semantic color (not used for Muted)
    TextRole              role    = TextRole::Body;
    Icon                  icon    = Icon::None;           // icon to the left of text
    std::optional<UiSize> size;                           // font size by UiSize metrics; unset — default
    bool                  wrap    = false;                // multiline text wrapped at container boundary (no icon)
};

// Displays a string of text at current layout position
void Text(const char* text, const TextOptions& options = {});
void Text(const std::string& text, const TextOptions& options = {});

// printf formatting to std::string: Text(Format("%.1f MB/s", v), {.role = TextRole::Muted})
#if defined(__GNUC__)
__attribute__((format(printf, 1, 2)))
#endif
std::string Format(const char* format, ...);

} // namespace OmniKit
