#pragma once

#include "ui/components/charts/ChartTypes.hpp"
#include "ui/components/charts/ChartStyle.hpp"
#include "ui/components/charts/ChartCore.hpp"
#include <optional>
#include <functional>
#include <vector>
#include <string>

class AnalysisChart {
public:
    AnalysisChart();
    explicit AnalysisChart(const ChartOptions& options, const ChartStyle* customStyle = nullptr);

    // 1. Конфигурация осей
    void SetXAxis(const std::string& label, const std::string& unit = "");
    void SetYAxis(const std::string& label, const std::string& unit = "");
    const std::string& GetXLabel() const { return m_xLabel; }
    const std::string& GetXUnit() const { return m_xUnit; }
    const std::string& GetYLabel() const { return m_yLabel; }
    const std::string& GetYUnit() const { return m_yUnit; }

    // 2. Отступы области графика (для размещения тиков и подписей)
    void SetPad(const ChartPad& pad) { m_pad = pad; }
    const ChartPad& GetPad() const { return m_pad; }

    // 3. Опции и стили графика
    void SetOptions(const ChartOptions& options) { m_options = options; }
    const ChartOptions& GetOptions() const { return m_options; }
    ChartOptions& GetOptions() { return m_options; }

    void SetMajorGrid(bool enable) { m_options.majorGrid = enable; }
    void SetMinorGrid(bool enable) { m_options.minorGrid = enable; }
    void SetCrosshair(bool enable) { m_options.crosshair = enable; }
    void SetStickyZero(bool enable) { m_options.stickyZero = enable; }
    void SetPanEnabled(bool enable) { m_options.panEnabled = enable; }
    void SetZoomEnabled(bool enable) { m_options.zoomEnabled = enable; }

    void SetStyle(const ChartStyle& style) { m_style = style; m_hasCustomStyle = true; }
    const ChartStyle& GetStyle() const { return m_style; }

    // 4. Серии данных (кривые)
    void AddSeries(ChartSeries series);
    void SetSeries(std::vector<ChartSeries> series);
    const std::vector<ChartSeries>& GetSeries() const { return m_series; }
    std::vector<ChartSeries>& GetSeries() { return m_series; }

    // 5. Оверлеи (маркеры и линии)
    void AddMarker(ChartMarker marker);
    void SetMarkers(std::vector<ChartMarker> markers);
    const std::vector<ChartMarker>& GetMarkers() const { return m_markers; }
    std::vector<ChartMarker>& GetMarkers() { return m_markers; }

    void AddLine(ChartLine line);
    void SetLines(std::vector<ChartLine> lines);
    const std::vector<ChartLine>& GetLines() const { return m_lines; }
    std::vector<ChartLine>& GetLines() { return m_lines; }

    // Очистка данных
    void ClearData(); // Очищает серии, маркеры и линии
    void Clear();     // Очищает данные и сбрасывает зум

    // 6. Масштаб и диапазон
    void SetZoom(double minX, double maxX);
    void ResetZoom();
    bool IsZoomed() const { return m_zoomWindowX.has_value(); }

    ChartBounds GetDataBounds() const;
    ChartBounds GetActiveBounds() const;

    // 7. Зонд / Наведение
    const ChartHoverInfo& GetHoverInfo() const { return m_hoverInfo; }
    void SetCustomTooltip(std::function<void(const ChartHoverInfo&)> callback) {
        m_customTooltip = std::move(callback);
    }

    // 8. Отрисовка виджета
    // Размер в px; 0 по оси — всё свободное место. Идентичность — сам объект графика
    void Render(ImVec2 sizePx = ImVec2(0.0f, 0.0f));

private:
    std::string m_xLabel;
    std::string m_xUnit;
    std::string m_yLabel;
    std::string m_yUnit;

    ChartPad m_pad;
    ChartOptions m_options;
    ChartStyle m_style = ChartStyle::Dark();
    bool m_hasCustomStyle = false;

    std::vector<ChartSeries> m_series;
    std::vector<ChartMarker> m_markers;
    std::vector<ChartLine> m_lines;

    std::optional<std::pair<double, double>> m_zoomWindowX;
    ChartHoverInfo m_hoverInfo;
    std::function<void(const ChartHoverInfo&)> m_customTooltip;

    mutable ChartBounds m_cachedDataBounds;
    mutable bool m_dataBoundsDirty = true;

    struct SeriesRenderCache {
        int lastColWidth = 0;
        double lastMinX = 0.0;
        double lastMaxX = 0.0;
        std::vector<ChartPoint> decimated;
    };
    std::vector<SeriesRenderCache> m_seriesCache;
    std::vector<ImVec2> m_screenPtsBuffer;

    void RenderGridAndAxes(ImDrawList* dl, const ChartCore::Projection& proj, const ChartStyle& style);
    void RenderSeriesCurves(ImDrawList* dl, const ChartCore::Projection& proj);
    void RenderOverlayLines(ImDrawList* dl, const ChartCore::Projection& proj);
    void RenderOverlayMarkers(ImDrawList* dl, const ChartCore::Projection& proj);
    void RenderCrosshairAndTooltip(ImDrawList* dl, const ChartCore::Projection& proj, const ChartStyle& style);
};
