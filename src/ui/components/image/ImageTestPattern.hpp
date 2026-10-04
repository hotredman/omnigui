#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace OmniKit {

// Synthetic 16-bit HDR test frame: an uneven illumination gradient with concentric
// frequency rings and detail. Matches drs-xview makeTestImage.
inline std::vector<uint16_t> GenerateTestPattern(int w = 1024, int h = 768) {
    std::vector<uint16_t> d(static_cast<size_t>(w) * h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float gx = static_cast<float>(x) / static_cast<float>(w);
            float gy = static_cast<float>(y) / static_cast<float>(h);
            float bg = 0.3f + 0.4f * gx + 0.2f * gy;

            float cx = (static_cast<float>(x) - w * 0.5f) / (w * 0.3f);
            float cy = (static_cast<float>(y) - h * 0.5f) / (h * 0.3f);
            float r = std::sqrt(cx * cx + cy * cy);
            float detail = 0.1f * std::sin(r * 20.0f) * std::exp(-r * 0.5f);

            float v = std::clamp(bg + detail, 0.0f, 1.0f);
            d[static_cast<size_t>(y) * w + x] = static_cast<uint16_t>(v * 65535.0f);
        }
    }
    return d;
}

} // namespace OmniKit
