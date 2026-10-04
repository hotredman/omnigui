#pragma once

#include "omnikit/charts/ChartTypes.hpp"
#include <vector>
#include <string>

namespace OmniKit {

namespace ChartCore {

// Generation of "nice" grid steps (1/2/5 x 10^n) aligned with the grid
std::vector<double> GenerateTicks(double min, double max, int count = 5);

// Generation of intermediate ticks for the minor grid (5x frequency)
std::vector<double> MinorTicks(double min, double max, int majorCount = 5);

// Format numeric axis label value
std::string FormatTickValue(double val);

// Coordinate projection between data space and screen pixels
struct Projection {
    ChartBounds bounds;
    ChartPad pad;
    ImVec2 origin{ 0.0f, 0.0f };
    ImVec2 canvasSize{ 0.0f, 0.0f };
    ImVec2 plotMin{ 0.0f, 0.0f };
    ImVec2 plotMax{ 0.0f, 0.0f };
    float plotWidth = 1.0f;
    float plotHeight = 1.0f;

    static Projection Create(const ChartBounds& bounds, const ChartPad& pad, ImVec2 origin, ImVec2 canvasSize);

    float ToScreenX(double x) const;
    float ToScreenY(double y) const;
    ImVec2 ToScreen(const ChartPoint& pt) const;

    double ToDataX(float screenX) const;
    double ToDataY(float screenY) const;
    ChartPoint ToData(const ImVec2& screenPt) const;
};

// Binary search for the closest point by X coordinate in O(log N)
int NearestIndexByX(const std::vector<ChartPoint>& points, double targetX);

// Liang-Barsky line clipping against rectangle [xmin, xmax] x [ymin, ymax]
bool ClipInfiniteLineToRect(double x1, double y1, double x2, double y2,
                            double xmin, double ymin, double xmax, double ymax,
                            double& outX1, double& outY1, double& outX2, double& outY2);

} // namespace ChartCore

} // namespace OmniKit
