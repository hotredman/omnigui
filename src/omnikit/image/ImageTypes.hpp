#pragma once

#include <cstdint>
#include <string>
#include "imgui_ext/recorder.h"

namespace OmniKit {

using ImageTextureDescriptor = ImGuiExt::ImageTextureDescriptor;

enum class ImageColormap : int {
    Grayscale = 0,
    Turbo     = 1
};

struct WindowLevelParams {
    float window   = 3443.0f; // Window width
    float level    = 2751.0f; // Level (center value)
    float gamma    = 1.0f;    // Midtone curve (>1 brightens, <1 darkens)
    bool  invert   = false;   // Invert polarity (black <-> white)
    ImageColormap colormap = ImageColormap::Grayscale;
};

struct ImageHoverInfo {
    int      pixelX   = -1;
    int      pixelY   = -1;
    uint16_t rawValue = 0;
    bool     valid    = false;
};

} // namespace OmniKit
