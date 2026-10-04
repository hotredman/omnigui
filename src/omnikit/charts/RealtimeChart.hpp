#pragma once

#include "omnikit/charts/ChartTypes.hpp"
#include "omnikit/charts/ChartStyle.hpp"
#include "omnikit/charts/ChartCore.hpp"
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

namespace OmniKit {

// ============================================================================
// Axis modes for real-time chart (Control panel / ControlView)
// ============================================================================
enum class RealtimeAxisPair {
    Force_Displacement = 0, // F - ΔL (Load - Crosshead displacement, kN - mm)
    Force_Time = 1,         // F - t (Load - Time, kN - s)
    Displacement_Time = 2,  // ΔL - t (Crosshead displacement - Time, mm - s)
    Force_Extensometer = 3  // F - ΔL_ext (Load - Extensometer, kN - mm)
};

// Real-time telemetry snapshot for streaming curve recording
struct RealtimeSample {
    double timeS = 0.0;
    double forceKN = 0.0;
    double displacementMm = 0.0;
    double extensometerMm = 0.0;

    RealtimeSample() = default;
    constexpr RealtimeSample(double t, double f, double d, double ext = 0.0)
        : timeS(t), forceKN(f), displacementMm(d), extensometerMm(ext) {}
};

// ============================================================================
// RealtimeChart: high-performance chart for real-time monitoring.
// Tailored for the machine control panel ("Control").
// No pan or zoom - features stable auto-scaling for streaming points.
// ============================================================================
class RealtimeChart {
public:
    RealtimeChart();

    // Axis mode selection
    void SetAxisPair(RealtimeAxisPair pair);
    RealtimeAxisPair GetAxisPair() const { return m_axisPair; }

    // Axis labels and measurement units
    void SetXAxis(std::string label, std::string unit = "");
    void SetYAxis(std::string label, std::string unit = "");
    const std::string& GetXLabel() const { return m_xLabel; }
    const std::string& GetXUnit() const { return m_xUnit; }
    const std::string& GetYLabel() const { return m_yLabel; }
    const std::string& GetYUnit() const { return m_yUnit; }

    // Streaming data ingestion
    void Clear();
    void AppendSample(double timeS, double forceKN, double displacementMm, double extensometerMm = 0.0);
    void AppendSample(const RealtimeSample& sample);
    void AppendPoint(double x, double y);

    size_t SampleCount() const { return m_samples.size(); }
    size_t PointCount() const { return m_points.size(); }
    bool Empty() const { return m_points.empty(); }

    const std::vector<RealtimeSample>& GetSamples() const { return m_samples; }
    const std::vector<ChartPoint>& GetPoints() const { return m_points; }

    // Data bounds with margin
    const ChartBounds& GetBounds() const { return m_bounds; }

    // Style configuration (Clean Architecture: independent from UiTheme)
    void SetStyle(const ChartStyle& style) { m_style = style; m_hasCustomStyle = true; }
    const ChartStyle& GetStyle() const { return m_style; }
    void SetLineColor(ImU32 color) { m_lineColor = color; }
    void SetLineThickness(float thickness) { m_lineThickness = thickness; }
    void SetHeadMarker(bool enable) { m_showHeadMarker = enable; }
    void SetPad(const ChartPad& pad) { m_pad = pad; }
    const ChartPad& GetPad() const { return m_pad; }

    // ImGui frame rendering
    // Size in px; 0 on an axis means all available space. Identity is the chart object itself
    void Render(ImVec2 sizePx = ImVec2(0.0f, 0.0f));

private:
    void UpdateAxisLabels();
    void RebuildPointsFromSamples();
    void UpdateBoundsWithPoint(double x, double y);
    std::pair<double, double> ExtractPoint(const RealtimeSample& s) const;

    RealtimeAxisPair m_axisPair = RealtimeAxisPair::Force_Displacement;
    std::string m_xLabel;
    std::string m_xUnit;
    std::string m_yLabel;
    std::string m_yUnit;

    std::vector<RealtimeSample> m_samples;
    std::vector<ChartPoint> m_points;
    std::vector<ImVec2> m_screenPoints;

    ChartBounds m_bounds;
    double m_rawMaxX = 0.0;
    double m_rawMaxY = 0.0;

    ChartPad m_pad;
    ChartStyle m_style = ChartStyle::Dark();
    bool m_hasCustomStyle = false;
    ImU32 m_lineColor = 0; // 0 — theme curve (ChartStyle::colLine)
    float m_lineThickness = 2.0f;
    bool m_showHeadMarker = true;
};

} // namespace OmniKit
