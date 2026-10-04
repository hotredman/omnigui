#pragma once

#include "ui/components/charts/ChartTypes.hpp"
#include "ui/components/charts/ChartStyle.hpp"
#include "ui/components/charts/ChartCore.hpp"
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

// ============================================================================
// Режимы осей для диаграммы реального времени (экран Пульт / ControlView)
// ============================================================================
enum class RealtimeAxisPair {
    Force_Displacement = 0, // F — ΔL (Нагрузка — Перемещение траверсы, кН — мм)
    Force_Time = 1,         // F — t (Нагрузка — Время, кН — с)
    Displacement_Time = 2,  // ΔL — t (Перемещение траверсы — Время, мм — с)
    Force_Extensometer = 3  // F — ΔL_ext (Нагрузка — Экстензометр, кН — мм)
};

// Срез телеметрии реального времени для потоковой записи кривой
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
// RealtimeChart: высокопроизводительный график для мониторинга в реальном времени.
// Заточен под экран управления машиной («Пульт»).
// Без панорамирования и зума — со стабильным автомасштабированием под поток точек.
// ============================================================================
class RealtimeChart {
public:
    RealtimeChart();

    // Выбор режима осей
    void SetAxisPair(RealtimeAxisPair pair);
    RealtimeAxisPair GetAxisPair() const { return m_axisPair; }

    // Подписи и единицы измерения осей
    void SetXAxis(std::string label, std::string unit = "");
    void SetYAxis(std::string label, std::string unit = "");
    const std::string& GetXLabel() const { return m_xLabel; }
    const std::string& GetXUnit() const { return m_xUnit; }
    const std::string& GetYLabel() const { return m_yLabel; }
    const std::string& GetYUnit() const { return m_yUnit; }

    // Потоковое добавление данных
    void Clear();
    void AppendSample(double timeS, double forceKN, double displacementMm, double extensometerMm = 0.0);
    void AppendSample(const RealtimeSample& sample);
    void AppendPoint(double x, double y);

    size_t SampleCount() const { return m_samples.size(); }
    size_t PointCount() const { return m_points.size(); }
    bool Empty() const { return m_points.empty(); }

    const std::vector<RealtimeSample>& GetSamples() const { return m_samples; }
    const std::vector<ChartPoint>& GetPoints() const { return m_points; }

    // Границы данных с учетом отступа
    const ChartBounds& GetBounds() const { return m_bounds; }

    // Style configuration (Clean Architecture: independent from UiTheme)
    void SetStyle(const ChartStyle& style) { m_style = style; m_hasCustomStyle = true; }
    const ChartStyle& GetStyle() const { return m_style; }
    void SetLineColor(ImU32 color) { m_lineColor = color; }
    void SetLineThickness(float thickness) { m_lineThickness = thickness; }
    void SetHeadMarker(bool enable) { m_showHeadMarker = enable; }
    void SetPad(const ChartPad& pad) { m_pad = pad; }
    const ChartPad& GetPad() const { return m_pad; }

    // Рендеринг в кадре ImGui
    // Размер в px; 0 по оси — всё свободное место. Идентичность — сам объект графика
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
    ImU32 m_lineColor = 0; // 0 — кривая темы (ChartStyle::colLine)
    float m_lineThickness = 2.0f;
    bool m_showHeadMarker = true;
};
