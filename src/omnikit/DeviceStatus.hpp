#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <string>
#include <vector>
#include <source_location>

namespace OmniKit {

enum class DeviceState {
    Disconnected, // Gray: device de-energized / offline
    Connecting,   // Yellow: polling, connecting, handshake
    Idle,         // Green: online, idle, and ready for operation
    Running,      // Blue: active operating routine
    Paused,       // Amber/orange: paused
    Fault         // Red: fault, overload, malfunction
};

// Device status panel options (designated initializers):
//
//     if (DeviceStatus("Cluster 07", state, {.status = "STANDBY", .action = "S"})) { ... }
//     if (DeviceStatus("Cluster 07", state)) { ... }   // power button (Connect / Disconnect)
struct DeviceStatusOptions {
    const char* status = nullptr;       // status text; nullptr — default for state
    const char* action = nullptr;       // short glyph (1–2 chars) in square button; nullptr — power icon
    bool        showAction = true;      // show action button
    bool        actionDisabled = false; // button is visible but disabled
    float       width = 0.0f;           // base px (scaled internally); 0 — from theme style
    UiKey       key = {};               // identity; defaults to deviceName

    // Device name candidates: if non-empty, width is sized automatically to the longest
    // name / status (preventing layout jitter on title change); width is then ignored
    std::vector<std::string> titleCandidates;

    const DeviceStatusStyle* style = nullptr;   // style override; nullptr — from theme
};

// Device status panel with an action button.
// Returns true if the action button was clicked.
bool DeviceStatus(const char* deviceName, DeviceState state, const DeviceStatusOptions& options = {},
                  std::source_location loc = std::source_location::current());

// Pre-computes required panel width accounting for complete geometry (in resolved px):
// measures candidate device names (titleCandidates), current name (currentTitle),
// all potential states ("DISCONNECTED", "CONNECTING", etc.), paddings, LED, and action button.
float DeviceStatusWidth(const std::vector<std::string>& titleCandidates = {},
                        const std::string& currentTitle = "",
                        const DeviceStatusStyle* style = nullptr);

// Default state label text
const char* DeviceStateText(DeviceState state);

// State LED indicator color
ImU32 DeviceStateColor(DeviceState state, const DeviceStatusStyle& style);

} // namespace OmniKit
