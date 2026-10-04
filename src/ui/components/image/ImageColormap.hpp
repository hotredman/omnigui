#pragma once

#include <cstdint>

namespace OmniKit {

// Returns a 256-entry RGB lookup table (768 bytes total)
// for Google's Turbo colormap (polynomial approximation by Anton Mikhailov / Matt Z).
const uint8_t* GetTurboLUT();

} // namespace OmniKit
