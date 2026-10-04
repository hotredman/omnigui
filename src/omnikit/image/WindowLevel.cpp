#include "WindowLevel.hpp"
#include "ImageColormap.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace OmniKit {

void BuildWindowLUT32(const WindowLevelParams& params, uint32_t* lut32) {
    if (!lut32) return;
    float lo = params.level - params.window * 0.5f;
    float hi = params.level + params.window * 0.5f;
    float denom = hi - lo;
    if (denom < 1.0f) denom = 1.0f;
    bool useGamma = (params.gamma != 1.0f && params.gamma > 0.0f);
    float invGamma = useGamma ? (1.0f / params.gamma) : 1.0f;
    const uint8_t* turbo = (params.colormap == ImageColormap::Turbo) ? GetTurboLUT() : nullptr;

    for (int v = 0; v < 65536; ++v) {
        float n = (static_cast<float>(v) - lo) / denom;
        n = std::clamp(n, 0.0f, 1.0f);
        if (useGamma) {
            n = std::pow(n, invGamma);
        }
        if (params.invert) {
            n = 1.0f - n;
        }
        int g = std::clamp(static_cast<int>(255.0f * n + 0.5f), 0, 255);
        if (turbo) {
            const uint8_t* c = &turbo[g * 3];
            // ABGR8888 in little-endian word: 0xAABBGGRR
            lut32[v] = (0xFFu << 24) | (uint32_t(c[2]) << 16) | (uint32_t(c[1]) << 8) | uint32_t(c[0]);
        } else {
            lut32[v] = (0xFFu << 24) | (uint32_t(g) << 16) | (uint32_t(g) << 8) | uint32_t(g);
        }
    }
}

bool ComputeAutoWindow(const uint16_t* data, int width, int height, float clipPercent, float& outWindow, float& outLevel) {
    if (!data || width <= 0 || height <= 0) return false;
    int ix = std::max(width / 100, 16);
    int iy = std::max(height / 100, 16);
    if (width <= ix * 2 || height <= iy * 2) {
        ix = 0;
        iy = 0;
    }

    std::vector<int> hist(65536, 0);
    int mn = 65535, mx = 0;
    for (int r = iy; r < height - iy; ++r) {
        const uint16_t* row = data + r * width;
        for (int c = ix; c < width - ix; ++c) {
            int v = row[c];
            hist[v]++;
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
    }
    if (mn >= mx) {
        outWindow = 1.0f;
        outLevel = static_cast<float>(mn);
        return true;
    }

    double S = 0.0;
    for (int v = mn; v <= mx; ++v) {
        int cv = hist[v];
        if (cv > 1) S += std::sqrt(static_cast<double>(cv));
    }
    double T = S * (2.0 * static_cast<double>(clipPercent)) / 100.0;

    // Black: skip steep head, then accumulate up to T
    int v = mn + 1;
    while (v < mx && hist[v - 1] >= 2 * hist[v]) v++;
    double acc = 0.0;
    int black_i = mn;
    while (v < mx) {
        int cv = hist[v];
        if (cv > 1) acc += std::sqrt(static_cast<double>(cv));
        if (acc > T) {
            black_i = v;
            break;
        }
        v++;
    }

    // White: mirror from the top
    v = mx - 1;
    while (v > mn && hist[v + 1] >= 2 * hist[v]) v--;
    acc = 0.0;
    int white_i = mx;
    while (v > mn) {
        int cv = hist[v];
        if (cv > 1) acc += std::sqrt(static_cast<double>(cv));
        if (acc > T) {
            white_i = v;
            break;
        }
        v--;
    }

    if (white_i <= black_i) white_i = black_i + 1;
    outWindow = static_cast<float>(white_i - black_i);
    outLevel = (static_cast<float>(black_i) + static_cast<float>(white_i)) * 0.5f;
    return true;
}

} // namespace OmniKit
