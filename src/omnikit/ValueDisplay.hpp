#pragma once

#include "omnikit/UiTheme.hpp"

namespace OmniKit {

// Value display parameters (designated initializers):
//
//     ValueDisplay(42.5, {.unit = "Hz"});
//     ValueDisplay(1280.0, {.unit = "points", .format = "%.0f", .size = UiSize::Small});
struct ValueDisplayOptions {
    const char* unit    = nullptr;    // measurement unit
    const char* format  = "%.2f";     // printf format (numeric value only)
    UiSize      size    = UiSize::Medium; // height and font size by UiSize scale (height matches input fields)
    const ValueDisplayStyle* style = nullptr;  // style override; nullptr — from theme

    // Internal mechanism for containers (Card, PresetGrid) that compute
    // geometry themselves: final px, 0 — auto (full default width / height). Not for application code.
    ImVec2      sizePx  = ImVec2(0.0f, 0.0f);
};

// Numeric value with measurement unit and formatting
void ValueDisplay(double value, const ValueDisplayOptions& options = {});

// String value with measurement unit (options.format is ignored)
void ValueDisplay(const char* text, const ValueDisplayOptions& options = {});

} // namespace OmniKit
