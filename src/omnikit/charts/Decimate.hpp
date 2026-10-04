#pragma once

#include "omnikit/charts/ChartTypes.hpp"
#include <vector>

namespace OmniKit {

namespace ChartCore {

// Min/max point decimation for pixel-column rendering:
// splits points into column bins and preserves min and max points in their original order.
// Guarantees preservation of signal peaks and valleys for any N without distortion.
std::vector<ChartPoint> DecimateMinMax(const std::vector<ChartPoint>& points, int columns);

} // namespace ChartCore

} // namespace OmniKit
