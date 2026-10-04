#pragma once

#include "ImageTypes.hpp"
#include <cstdint>

namespace OmniKit {

// Precomputes a 65536-entry 32-bit pixel LUT (ABGR8888 little-endian 0xAABBGGRR)
// mapping each raw 16-bit uint16 value to final display color.
void BuildWindowLUT32(const WindowLevelParams& params, uint32_t* lut32 /* 65536 entries */);

// Automatically computes optimal Window and Level parameters based on the 16-bit image histogram
// using a sqrt-weighted percentile clip with steep-head rejection (the DiSoft autolevel algorithm).
// clipPercent is typically 0.8% - 1.0%.
bool ComputeAutoWindow(const uint16_t* data, int width, int height, float clipPercent, float& outWindow, float& outLevel);

} // namespace OmniKit
