#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <string>
#include <vector>
#include <source_location>

namespace OmniKit {

// Preset grid options (designated initializers):
//
//     PresetGrid(rate, {10.0f, 50.0f, 100.0f}, {.unit = "Hz", .columns = 3});
struct PresetGridOptions {
    const char* unit          = nullptr;   // Unit displayed on the readout
    int         columns       = 5;
    UiSize      size          = UiSize::Medium;  // Button and display height on the UiSize scale
    bool        showDisplay   = true;      // Current value readout above grid
    const char* displayFormat = "%.1f";    // Readout value format string
    const char* buttonFormat  = "%.4g";    // Preset button label format string
    UiKey       key           = {};        // Identity; default uses call-site location (loc)
    const PresetGridStyle* style = nullptr;  // Style override; nullptr uses theme

    // Internal mechanism for containers (Card) managing their own layout geometry:
    // final px (width, readout height); 0 for auto. Not intended for application code.
    ImVec2      sizePx        = ImVec2(0.0f, 0.0f);
};

// Preset selector: value readout + button grid. Returns true if value changed
bool PresetGrid(float& value, const std::vector<float>& presets, const PresetGridOptions& options = {},
                std::source_location loc = std::source_location::current());
bool PresetGrid(double& value, const std::vector<double>& presets, const PresetGridOptions& options = {},
                std::source_location loc = std::source_location::current());

} // namespace OmniKit
