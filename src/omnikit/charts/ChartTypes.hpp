#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <cstdint>

namespace OmniKit {

enum class LineStyle {
    Solid,  // Solid line
    Dashed, // Dashed line
    Dotted  // Dotted line
};

enum class MarkerShape {
    Diamond,  // ◆ Diamond
    Circle,   // ● Circle
    Square,   // ■ Square
    Triangle, // ▲ Triangle
    Cross     // ✖ Cross
};

struct ChartPoint {
    double x = 0.0;
    double y = 0.0;

    ChartPoint() = default;
    constexpr ChartPoint(double x_, double y_) : x(x_), y(y_) {}
};

struct ChartBounds {
    double minX = 0.0;
    double maxX = 1.0;
    double minY = 0.0;
    double maxY = 1.0;

    bool IsValid() const {
        return maxX > minX && maxY > minY;
    }
};

struct ChartPad {
    float top = 30.0f;
    float right = 30.0f;
    float bottom = 56.0f;
    float left = 75.0f;
};

struct ChartOptions {
    bool majorGrid   = true; // Major grid along major axis divisions
    bool minorGrid   = true; // Minor sub-grid (5x)
    bool crosshair   = true; // Probe crosshair on cursor hover
    bool stickyZero  = true; // Smart zoom snapping X=0 in elastic region
    bool panEnabled  = true; // Whether panning via LMB drag is enabled
    bool zoomEnabled = true; // Whether mouse wheel zooming is enabled
};

// ============================================================================
// Data series (curve)
// ============================================================================
struct ChartSeries {
    std::string id;
    std::string label;
    std::string sublabel;
    std::vector<ChartPoint> points;

    ImU32 color = 0;  // 0 — theme curve (ChartStyle::colLine)
    float thickness = 2.0f;
    LineStyle style = LineStyle::Solid;
};

// ============================================================================
// Auxiliary line (elastic chord, parallel, threshold)
// ============================================================================
struct ChartLine {
    std::string id;
    std::string label;
    std::string sublabel;
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;

    ImU32 color = 0;  // 0 — theme curve (ChartStyle::colLine)
    float thickness = 1.5f;
    LineStyle style = LineStyle::Dashed;
    bool infinite = false;
    float labelPosition = 0.5f; // 0.0 .. 1.0 along line segment
};

// ============================================================================
// Key point marker (peak, yield point, rupture point)
// ============================================================================
struct ChartMarker {
    std::string id;
    std::string label;
    std::string sublabel;
    double x = 0.0;
    double y = 0.0;

    ImU32 color = 0;  // 0 — theme curve (ChartStyle::colLine)
    float size = 6.0f;
    MarkerShape shape = MarkerShape::Diamond;
    bool projectX = true; // Dashed projection to X axis (downwards)
    bool projectY = true; // Dashed projection to Y axis (leftwards)
};

// ============================================================================
// Cursor hover event (probe)
// ============================================================================
struct ChartHoverInfo {
    bool hasHover = false;
    int seriesIndex = -1;
    size_t pointIndex = 0;
    double dataX = 0.0;
    double dataY = 0.0;
    ImVec2 screenPos{ 0.0f, 0.0f };
};

} // namespace OmniKit
