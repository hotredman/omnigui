#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/TextSuggest.hpp"
#include "omnikit/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <optional>
#include <source_location>

namespace OmniKit {

// Input field parameters (designated initializers):
//
//     InputField(rate, {.label = "Rate", .unit = "MB/s"});
//     InputField(name, {.label = "Name", .hint = "Enter a name"});
//
// Identity is derived from key, otherwise from call site (loc) and label.
struct InputFieldOptions {
    const char* label   = nullptr;              // label above the field
    const char* unit    = nullptr;              // measurement unit inside the field on the right
    const char* format  = "%.2f";               // printf format for float/double
    const char* hint    = " — ";                // placeholder text for empty field (optional)
    UiVariant   variant = UiVariant::Default;   // border color (e.g. Warning for modified value)
    UiSize      size    = UiSize::Medium;       // field height
    float       width   = 0.0f;                 // base px (pre-scale); 0 — default width; Fill — full width
    UiKey       key     = {};                   // explicit identity
    ImVec2      sizePx  = {};                   // for containers with computed geometry: final px, overrides width and size
    const TextSuggest* suggest = nullptr;       // suggestions below the field (std::string only); list is built by caller
};

// All overloads return true in the frame when the value changes.
// Floating-point number with embedded measurement unit
bool InputField(float& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
bool InputField(double& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
// Integer number
bool InputField(int& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
// Text field with placeholder (options.hint)
bool InputField(std::string& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
// Optional numbers (nullopt is displayed as options.hint)
bool InputField(std::optional<float>& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
bool InputField(std::optional<double>& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());

} // namespace OmniKit
