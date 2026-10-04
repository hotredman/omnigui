#pragma once

#include "ImageTypes.hpp"
#include "ImageStyle.hpp"
#include <vector>
#include <cstdint>

namespace OmniKit {

class ImageHistogram {
public:
    ImageHistogram();

    void Compute(const uint16_t* data, int width, int height);

    // Style configuration (Clean Architecture: independent from UiTheme)
    void SetStyle(const ImageHistogramStyle& style) { m_style = style; m_hasCustomStyle = true; }
    const ImageHistogramStyle& GetStyle() const { return m_style; }

    // Returns true if W/L values were changed via dragging
    bool Render(const WindowLevelParams& currentParams, WindowLevelParams& outParams, float width = -1.0f, float height = 80.0f, const ImageHistogramStyle* style = nullptr);

    float GetMinValue() const { return m_minValue; }
    float GetMaxValue() const { return m_maxValue; }
    const std::vector<float>& GetBins() const { return m_bins; }
    bool HasData() const { return !m_bins.empty() && m_maxValue > m_minValue; }

private:
    std::vector<float> m_bins; // 256 counts
    float m_minValue = 0.0f;
    float m_maxValue = 65535.0f;

    // Interaction state: 0 = none, 1 = low edge, 2 = high edge, 3 = band move
    int m_dragMode = 0;
    float m_dragLo = 0.0f;
    float m_dragHi = 0.0f;
    float m_dragVal = 0.0f;

    ImageHistogramStyle m_style = ImageHistogramStyle::Dark();
    bool m_hasCustomStyle = false;
};

} // namespace OmniKit
