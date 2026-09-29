#include "ui/components/charts/AnalysisChart.hpp"
#include "ui/components/charts/Decimate.hpp"
#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>
#include <cstdio>

static void DrawDashedLine(ImDrawList* dl, ImVec2 p1, ImVec2 p2, ImU32 col, float thickness, float dash, float gap) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float len = std::hypot(dx, dy);
    if (len <= 0.001f) return;

    float nx = dx / len;
    float ny = dy / len;
    float step = dash + gap;
    float current = 0.0f;

    while (current < len) {
        float segLen = std::min(dash, len - current);
        ImVec2 segP1(p1.x + nx * current, p1.y + ny * current);
        ImVec2 segP2(segP1.x + nx * segLen, segP1.y + ny * segLen);
        dl->AddLine(segP1, segP2, col, thickness);
        current += step;
    }
}

static void DrawMarkerShape(ImDrawList* dl, ImVec2 center, float size, MarkerShape shape, ImU32 fillCol, ImU32 outlineCol) {
    float s = size;
    switch (shape) {
        case MarkerShape::Circle: {
            dl->AddCircleFilled(center, s, fillCol);
            dl->AddCircle(center, s + 1.0f, outlineCol, 16, 1.5f);
            break;
        }
        case MarkerShape::Diamond: {
            ImVec2 pts[4] = {
                ImVec2(center.x, center.y - s * 1.2f),
                ImVec2(center.x + s * 1.2f, center.y),
                ImVec2(center.x, center.y + s * 1.2f),
                ImVec2(center.x - s * 1.2f, center.y)
            };
            dl->AddConvexPolyFilled(pts, 4, fillCol);
            dl->AddPolyline(pts, 4, outlineCol, ImDrawFlags_Closed, 1.5f);
            break;
        }
        case MarkerShape::Square: {
            ImVec2 pMin(center.x - s, center.y - s);
            ImVec2 pMax(center.x + s, center.y + s);
            dl->AddRectFilled(pMin, pMax, fillCol);
            dl->AddRect(pMin, pMax, outlineCol, 0.0f, 0, 1.5f);
            break;
        }
        case MarkerShape::Triangle: {
            ImVec2 pts[3] = {
                ImVec2(center.x, center.y - s * 1.2f),
                ImVec2(center.x + s * 1.1f, center.y + s * 0.8f),
                ImVec2(center.x - s * 1.1f, center.y + s * 0.8f)
            };
            dl->AddTriangleFilled(pts[0], pts[1], pts[2], fillCol);
            dl->AddTriangle(pts[0], pts[1], pts[2], outlineCol, 1.5f);
            break;
        }
        case MarkerShape::Cross: {
            dl->AddLine(ImVec2(center.x - s, center.y - s), ImVec2(center.x + s, center.y + s), outlineCol, 2.0f);
            dl->AddLine(ImVec2(center.x - s, center.y + s), ImVec2(center.x + s, center.y - s), outlineCol, 2.0f);
            break;
        }
    }
}

AnalysisChart::AnalysisChart()
    : AnalysisChart(ChartOptions(), nullptr)
{
}

AnalysisChart::AnalysisChart(const ChartOptions& options, const ChartStyle* customStyle)
    : m_options(options)
{
    if (customStyle) {
        m_style = *customStyle;
        m_hasCustomStyle = true;
    }
}

const ChartStyle& AnalysisChart::GetStyle() const {
    if (m_hasCustomStyle) return m_style;
    return UiTheme::Get().chart;
}

void AnalysisChart::SetXAxis(const std::string& label, const std::string& unit) {
    m_xLabel = label;
    m_xUnit = unit;
}

void AnalysisChart::SetYAxis(const std::string& label, const std::string& unit) {
    m_yLabel = label;
    m_yUnit = unit;
}

void AnalysisChart::AddSeries(ChartSeries series) {
    m_series.push_back(std::move(series));
    m_dataBoundsDirty = true;
    m_seriesCache.clear();
}

void AnalysisChart::SetSeries(std::vector<ChartSeries> series) {
    m_series = std::move(series);
    m_dataBoundsDirty = true;
    m_seriesCache.clear();
}

void AnalysisChart::AddMarker(ChartMarker marker) {
    m_markers.push_back(std::move(marker));
    m_dataBoundsDirty = true;
}

void AnalysisChart::SetMarkers(std::vector<ChartMarker> markers) {
    m_markers = std::move(markers);
    m_dataBoundsDirty = true;
}

void AnalysisChart::AddLine(ChartLine line) {
    m_lines.push_back(std::move(line));
    m_dataBoundsDirty = true;
}

void AnalysisChart::SetLines(std::vector<ChartLine> lines) {
    m_lines = std::move(lines);
    m_dataBoundsDirty = true;
}

void AnalysisChart::ClearData() {
    m_series.clear();
    m_markers.clear();
    m_lines.clear();
    m_hoverInfo = ChartHoverInfo();
    m_dataBoundsDirty = true;
    m_seriesCache.clear();
}

void AnalysisChart::Clear() {
    ClearData();
    ResetZoom();
}

void AnalysisChart::SetZoom(double minX, double maxX) {
    if (maxX > minX) {
        m_zoomWindowX = { minX, maxX };
    }
}

void AnalysisChart::ResetZoom() {
    m_zoomWindowX = std::nullopt;
}

ChartBounds AnalysisChart::GetDataBounds() const {
    if (!m_dataBoundsDirty) {
        return m_cachedDataBounds;
    }

    ChartBounds b;
    bool hasData = false;

    for (const auto& s : m_series) {
        for (const auto& pt : s.points) {
            if (!hasData) {
                b.minX = b.maxX = pt.x;
                b.minY = b.maxY = pt.y;
                hasData = true;
            } else {
                if (pt.x < b.minX) b.minX = pt.x;
                if (pt.x > b.maxX) b.maxX = pt.x;
                if (pt.y < b.minY) b.minY = pt.y;
                if (pt.y > b.maxY) b.maxY = pt.y;
            }
        }
    }

    for (const auto& m : m_markers) {
        if (!hasData) {
            b.minX = b.maxX = m.x;
            b.minY = b.maxY = m.y;
            hasData = true;
        } else {
            if (m.x < b.minX) b.minX = m.x;
            if (m.x > b.maxX) b.maxX = m.x;
            if (m.y < b.minY) b.minY = m.y;
            if (m.y > b.maxY) b.maxY = m.y;
        }
    }

    for (const auto& l : m_lines) {
        if (l.infinite) continue;
        if (!hasData) {
            b.minX = std::min(l.x1, l.x2);
            b.maxX = std::max(l.x1, l.x2);
            b.minY = std::min(l.y1, l.y2);
            b.maxY = std::max(l.y1, l.y2);
            hasData = true;
        } else {
            if (l.x1 < b.minX) b.minX = l.x1;
            if (l.x2 < b.minX) b.minX = l.x2;
            if (l.x1 > b.maxX) b.maxX = l.x1;
            if (l.x2 > b.maxX) b.maxX = l.x2;
            if (l.y1 < b.minY) b.minY = l.y1;
            if (l.y2 < b.minY) b.minY = l.y2;
            if (l.y1 > b.maxY) b.maxY = l.y1;
            if (l.y2 > b.maxY) b.maxY = l.y2;
        }
    }

    if (!hasData) {
        m_cachedDataBounds = ChartBounds{ 0.0, 1.0, 0.0, 1.0 };
        m_dataBoundsDirty = false;
        return m_cachedDataBounds;
    }

    if (b.minX > 0.0) b.minX = 0.0;
    if (b.minY > 0.0) b.minY = 0.0;

    if (b.maxX <= b.minX) b.maxX = b.minX + 1.0;
    if (b.maxY <= b.minY) b.maxY = b.minY + 1.0;

    m_cachedDataBounds = b;
    m_dataBoundsDirty = false;
    return m_cachedDataBounds;
}

ChartBounds AnalysisChart::GetActiveBounds() const {
    ChartBounds dataB = GetDataBounds();
    ChartBounds activeB;

    if (m_zoomWindowX.has_value()) {
        activeB.minX = m_zoomWindowX->first;
        activeB.maxX = m_zoomWindowX->second;

        // Подгоняем ось Y под максимальное видимое значение в текущем окне X
        double visibleMaxY = 0.0;
        bool hasVisible = false;

        for (const auto& s : m_series) {
            for (const auto& pt : s.points) {
                if (pt.x >= activeB.minX && pt.x <= activeB.maxX) {
                    if (!hasVisible || pt.y > visibleMaxY) {
                        visibleMaxY = pt.y;
                        hasVisible = true;
                    }
                }
            }
        }

        for (const auto& m : m_markers) {
            if (m.x >= activeB.minX && m.x <= activeB.maxX) {
                if (!hasVisible || m.y > visibleMaxY) {
                    visibleMaxY = m.y;
                    hasVisible = true;
                }
            }
        }

        if (!hasVisible || visibleMaxY <= 0.0) {
            visibleMaxY = dataB.maxY;
        }

        activeB.minY = 0.0;
        activeB.maxY = visibleMaxY * 1.08;
    } else {
        activeB.minX = dataB.minX;
        activeB.maxX = dataB.maxX + (dataB.maxX - dataB.minX) * 0.05;
        activeB.minY = 0.0;
        activeB.maxY = dataB.maxY * 1.08;
    }

    if (activeB.maxY <= activeB.minY) {
        activeB.maxY = activeB.minY + 1.0;
    }

    return activeB;
}

void AnalysisChart::Render(const char* id, ImVec2 size) {
    const UiTheme& theme = UiTheme::Get();
    ChartStyle style = GetStyle();

    ImVec2 canvasSize = size;
    if (canvasSize.x <= 0.0f) canvasSize.x = ImGui::GetContentRegionAvail().x;
    if (canvasSize.y <= 0.0f) canvasSize.y = ImGui::GetContentRegionAvail().y;

    canvasSize.x = std::max(100.0f, canvasSize.x);
    canvasSize.y = std::max(80.0f, canvasSize.y);

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(id, canvasSize, ImGuiButtonFlags_MouseButtonLeft);

    bool isHovered = ImGui::IsItemHovered();
    bool isActive = ImGui::IsItemActive();

    ChartBounds dataBounds = GetDataBounds();
    ChartBounds activeBounds = GetActiveBounds();

    ChartPad scaledPad = m_pad;
    scaledPad.left = theme.Scale(m_pad.left);
    scaledPad.right = theme.Scale(m_pad.right);
    scaledPad.top = theme.Scale(m_pad.top);
    scaledPad.bottom = theme.Scale(m_pad.bottom);

    ChartCore::Projection proj = ChartCore::Projection::Create(activeBounds, scaledPad, origin, canvasSize);

    // =========================================================================
    // Обработка интерактивности (Zoom, Pan, Reset)
    // =========================================================================
    ImVec2 mousePos = ImGui::GetMousePos();
    bool mouseInPlot = (mousePos.x >= proj.plotMin.x && mousePos.x <= proj.plotMax.x &&
                        mousePos.y >= proj.plotMin.y && mousePos.y <= proj.plotMax.y);

    bool boundsChanged = false;

    // 1. Зум колесом мыши
    if (isHovered && mouseInPlot && m_options.zoomEnabled) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (std::abs(wheel) > 0.01f) {
            double cursorFrac = static_cast<double>(mousePos.x - proj.plotMin.x) / proj.plotWidth;
            cursorFrac = std::clamp(cursorFrac, 0.0, 1.0);

            bool isAtOrigin = std::abs(activeBounds.minX - dataBounds.minX) < 1e-5;
            bool isSmartZero = m_options.stickyZero && (ImGui::GetIO().KeyCtrl || (isAtOrigin && cursorFrac <= 0.40));
            double frac = isSmartZero ? 0.0 : cursorFrac;

            double currentSpan = activeBounds.maxX - activeBounds.minX;
            double factor = std::exp(-wheel * 0.15);
            double newSpan = currentSpan * factor;

            double fullSpan = (dataBounds.maxX - dataBounds.minX) * 1.05;
            double minSpan = std::max(1e-4, fullSpan * 0.005);

            if (newSpan < minSpan) newSpan = minSpan;
            if (newSpan >= fullSpan) {
                m_zoomWindowX = std::nullopt;
            } else {
                double newMinX = 0.0;
                double newMaxX = 0.0;
                if (isSmartZero) {
                    newMinX = dataBounds.minX;
                    newMaxX = newMinX + newSpan;
                } else {
                    double cursorDataX = activeBounds.minX + frac * currentSpan;
                    newMinX = cursorDataX - frac * newSpan;
                    newMaxX = newMinX + newSpan;

                    if (newMinX < dataBounds.minX) {
                        newMinX = dataBounds.minX;
                        newMaxX = newMinX + newSpan;
                    }
                    double maxBoundX = dataBounds.maxX * 1.05;
                    if (newMaxX > maxBoundX) {
                        newMaxX = maxBoundX;
                        newMinX = std::max(dataBounds.minX, maxBoundX - newSpan);
                    }
                }
                m_zoomWindowX = { newMinX, newMaxX };
            }
            boundsChanged = true;
        }
    }

    // 2. Панорамирование (Drag ЛКМ)
    if (m_options.panEnabled && m_zoomWindowX.has_value() && isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 2.0f)) {
        ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

        double span = m_zoomWindowX->second - m_zoomWindowX->first;
        double dxData = -static_cast<double>(dragDelta.x / proj.plotWidth) * span;

        double newMinX = m_zoomWindowX->first + dxData;
        double newMaxX = m_zoomWindowX->second + dxData;
        double maxBoundX = dataBounds.maxX * 1.05;

        if (newMinX < dataBounds.minX) {
            newMinX = dataBounds.minX;
            newMaxX = newMinX + span;
        }
        if (newMaxX > maxBoundX) {
            newMaxX = maxBoundX;
            newMinX = std::max(dataBounds.minX, maxBoundX - span);
        }

        m_zoomWindowX = { newMinX, newMaxX };
        boundsChanged = true;
    }

    // 3. Сброс зума по двойному клику
    if (isHovered && mouseInPlot && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        ResetZoom();
        boundsChanged = true;
    }

    if (boundsChanged) {
        // Пересчитываем проекцию с обновлённым окном зума
        activeBounds = GetActiveBounds();
        proj = ChartCore::Projection::Create(activeBounds, scaledPad, origin, canvasSize);
    }

    // =========================================================================
    // Отрисовка
    // =========================================================================
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // 1. Фон и рамка
    dl->AddRectFilled(origin, ImVec2(origin.x + canvasSize.x, origin.y + canvasSize.y),
                      style.colBg, theme.Scale(style.cornerRadius));
    dl->AddRect(origin, ImVec2(origin.x + canvasSize.x, origin.y + canvasSize.y),
                style.colBorder, theme.Scale(style.cornerRadius), 0, style.borderSize);

    // 2. Сетка и оси
    RenderGridAndAxes(dl, proj, style);

    // 3. Серии и оверлеи внутри области отсечения графика
    dl->PushClipRect(proj.plotMin, proj.plotMax, true);
    RenderSeriesCurves(dl, proj);
    RenderOverlayLines(dl, proj);
    RenderOverlayMarkers(dl, proj);
    dl->PopClipRect();

    // 4. Интерактивный зонд (перекрестие и тултип)
    if (isHovered && mouseInPlot && m_options.crosshair) {
        RenderCrosshairAndTooltip(dl, proj, style);
    } else {
        m_hoverInfo = ChartHoverInfo();
    }
}

void AnalysisChart::RenderGridAndAxes(ImDrawList* dl, const ChartCore::Projection& proj, const ChartStyle& style) {
    const UiTheme& theme = UiTheme::Get();

    std::vector<double> xMajorTicks = ChartCore::GenerateTicks(proj.bounds.minX, proj.bounds.maxX, 5);
    std::vector<double> yMajorTicks = ChartCore::GenerateTicks(proj.bounds.minY, proj.bounds.maxY, 5);

    // Минорная сетка (x5)
    if (m_options.minorGrid) {
        std::vector<double> xMinorTicks = ChartCore::MinorTicks(proj.bounds.minX, proj.bounds.maxX, 5);
        for (double xVal : xMinorTicks) {
            float sx = proj.ToScreenX(xVal);
            if (sx >= proj.plotMin.x && sx <= proj.plotMax.x) {
                dl->AddLine(ImVec2(sx, proj.plotMin.y), ImVec2(sx, proj.plotMax.y), style.colMinorGrid, style.minorGridWidth);
            }
        }
        std::vector<double> yMinorTicks = ChartCore::MinorTicks(proj.bounds.minY, proj.bounds.maxY, 5);
        for (double yVal : yMinorTicks) {
            float sy = proj.ToScreenY(yVal);
            if (sy >= proj.plotMin.y && sy <= proj.plotMax.y) {
                dl->AddLine(ImVec2(proj.plotMin.x, sy), ImVec2(proj.plotMax.x, sy), style.colMinorGrid, style.minorGridWidth);
            }
        }
    }

    // Мажорная сетка и числовые подписи
    theme.PushFont(theme.fontRegular, style.tickFontSize);

    // Горизонтальные линии Y и числовые метки
    for (double yVal : yMajorTicks) {
        float sy = proj.ToScreenY(yVal);
        if (sy >= proj.plotMin.y - 1.0f && sy <= proj.plotMax.y + 1.0f) {
            if (m_options.majorGrid) {
                dl->AddLine(ImVec2(proj.plotMin.x, sy), ImVec2(proj.plotMax.x, sy), style.colMajorGrid, style.majorGridWidth);
            }
            std::string text = ChartCore::FormatTickValue(yVal);
            ImVec2 txtSz = ImGui::CalcTextSize(text.c_str());
            dl->AddText(ImVec2(proj.plotMin.x - txtSz.x - theme.Scale(6.0f), sy - txtSz.y * 0.5f),
                        style.colTickText, text.c_str());
        }
    }

    // Вертикальные линии X и числовые метки
    for (double xVal : xMajorTicks) {
        float sx = proj.ToScreenX(xVal);
        if (sx >= proj.plotMin.x - 1.0f && sx <= proj.plotMax.x + 1.0f) {
            if (m_options.majorGrid) {
                dl->AddLine(ImVec2(sx, proj.plotMin.y), ImVec2(sx, proj.plotMax.y), style.colMajorGrid, style.majorGridWidth);
            }
            std::string text = ChartCore::FormatTickValue(xVal);
            ImVec2 txtSz = ImGui::CalcTextSize(text.c_str());
            dl->AddText(ImVec2(sx - txtSz.x * 0.5f, proj.plotMax.y + theme.Scale(6.0f)),
                        style.colTickText, text.c_str());
        }
    }

    // Рамка области графика
    dl->AddRect(proj.plotMin, proj.plotMax, style.colAxis, 0.0f, 0, 1.0f);

    theme.PopFont();

    // Заголовки осей
    theme.PushFont(theme.fontMedium, style.axisTitleFontSize);

    // Заголовок оси Y (сверху слева)
    if (!m_yLabel.empty()) {
        std::string yTitle = m_yUnit.empty() ? m_yLabel : (m_yLabel + " (" + m_yUnit + ")");
        float yPos = (proj.plotMin.y - proj.origin.y >= theme.Scale(24.0f))
                         ? (proj.origin.y + theme.Scale(3.0f))
                         : (proj.plotMin.y + theme.Scale(4.0f));
        dl->AddText(ImVec2(proj.plotMin.x + theme.Scale(6.0f), yPos),
                    style.colAxisTitle, yTitle.c_str());
    }

    // Заголовок оси X (внизу справа)
    if (!m_xLabel.empty()) {
        std::string xTitle = m_xUnit.empty() ? m_xLabel : (m_xLabel + " (" + m_xUnit + ")");
        ImVec2 xSz = ImGui::CalcTextSize(xTitle.c_str());
        dl->AddText(ImVec2(proj.plotMax.x - xSz.x, proj.plotMax.y + theme.Scale(28.0f)),
                    style.colAxisTitle, xTitle.c_str());
    }

    theme.PopFont();
}

void AnalysisChart::RenderSeriesCurves(ImDrawList* dl, const ChartCore::Projection& proj) {
    int colWidth = static_cast<int>(proj.plotWidth);
    if (colWidth < 20) colWidth = 20;

    if (m_seriesCache.size() != m_series.size()) {
        m_seriesCache.resize(m_series.size());
    }

    for (size_t sIdx = 0; sIdx < m_series.size(); ++sIdx) {
        const auto& s = m_series[sIdx];
        if (s.points.empty()) continue;

        auto& cache = m_seriesCache[sIdx];
        if (cache.decimated.empty() || cache.lastColWidth != colWidth ||
            cache.lastMinX != proj.bounds.minX || cache.lastMaxX != proj.bounds.maxX)
        {
            if (m_zoomWindowX.has_value()) {
                std::vector<ChartPoint> visible;
                visible.reserve(s.points.size());
                for (const auto& pt : s.points) {
                    if (pt.x >= proj.bounds.minX && pt.x <= proj.bounds.maxX) {
                        visible.push_back(pt);
                    }
                }
                cache.decimated = ChartCore::DecimateMinMax(visible, colWidth);
            } else {
                cache.decimated = ChartCore::DecimateMinMax(s.points, colWidth);
            }
            cache.lastColWidth = colWidth;
            cache.lastMinX = proj.bounds.minX;
            cache.lastMaxX = proj.bounds.maxX;
        }

        if (cache.decimated.size() < 2) continue;

        m_screenPtsBuffer.clear();
        m_screenPtsBuffer.reserve(cache.decimated.size());
        for (const auto& pt : cache.decimated) {
            m_screenPtsBuffer.push_back(proj.ToScreen(pt));
        }

        const ImU32 seriesColor = s.color ? s.color : GetStyle().colLine;
        if (s.style == LineStyle::Solid) {
            dl->AddPolyline(m_screenPtsBuffer.data(), static_cast<int>(m_screenPtsBuffer.size()), seriesColor, 0, s.thickness);
        } else {
            float dashLen = (s.style == LineStyle::Dotted) ? 2.0f : 6.0f;
            float gapLen = (s.style == LineStyle::Dotted) ? 3.0f : 4.0f;
            for (size_t i = 1; i < m_screenPtsBuffer.size(); ++i) {
                DrawDashedLine(dl, m_screenPtsBuffer[i - 1], m_screenPtsBuffer[i], seriesColor, s.thickness, dashLen, gapLen);
            }
        }
    }
}

void AnalysisChart::RenderOverlayLines(ImDrawList* dl, const ChartCore::Projection& proj) {
    const UiTheme& theme = UiTheme::Get();

    for (const auto& l : m_lines) {
        double x1 = l.x1, y1 = l.y1;
        double x2 = l.x2, y2 = l.y2;

        if (l.infinite) {
            double cx1, cy1, cx2, cy2;
            if (!ChartCore::ClipInfiniteLineToRect(x1, y1, x2, y2,
                                                  proj.bounds.minX, proj.bounds.minY,
                                                  proj.bounds.maxX, proj.bounds.maxY,
                                                  cx1, cy1, cx2, cy2))
            {
                continue;
            }
            x1 = cx1; y1 = cy1;
            x2 = cx2; y2 = cy2;
        }

        ImVec2 p1 = proj.ToScreen(ChartPoint(x1, y1));
        ImVec2 p2 = proj.ToScreen(ChartPoint(x2, y2));

        const ImU32 lineColor = l.color ? l.color : GetStyle().colLine;
        if (l.style == LineStyle::Solid) {
            dl->AddLine(p1, p2, lineColor, l.thickness);
        } else {
            float dashLen = (l.style == LineStyle::Dotted) ? 2.0f : 6.0f;
            float gapLen = (l.style == LineStyle::Dotted) ? 3.0f : 4.0f;
            DrawDashedLine(dl, p1, p2, lineColor, l.thickness, dashLen, gapLen);
        }

        // Подпись линии
        if (!l.label.empty()) {
            float t = std::clamp(l.labelPosition, 0.0f, 1.0f);
            ImVec2 lblPos(p1.x + (p2.x - p1.x) * t + theme.Scale(8.0f),
                          p1.y + (p2.y - p1.y) * t - theme.Scale(12.0f));

            theme.PushFont(theme.fontMedium, 13.0f);
            dl->AddText(lblPos, lineColor, l.label.c_str());
            theme.PopFont();
        }
    }
}

void AnalysisChart::RenderOverlayMarkers(ImDrawList* dl, const ChartCore::Projection& proj) {
    const UiTheme& theme = UiTheme::Get();

    for (const auto& m : m_markers) {
        if (m.x < proj.bounds.minX || m.x > proj.bounds.maxX ||
            m.y < proj.bounds.minY || m.y > proj.bounds.maxY)
        {
            continue;
        }

        ImVec2 ptPos = proj.ToScreen(ChartPoint(m.x, m.y));

        // Пунктирные проекции на оси
        const ChartStyle& style = GetStyle();
        const ImU32 markerColor = m.color ? m.color : style.colLine;
        const ImU32 projCol = style.colProjection;
        if (m.projectX) {  // на ось X (вниз)
            DrawDashedLine(dl, ptPos, ImVec2(ptPos.x, proj.plotMax.y), projCol, 1.0f, 4.0f, 3.0f);
        }
        if (m.projectY) {  // на ось Y (влево)
            DrawDashedLine(dl, ptPos, ImVec2(proj.plotMin.x, ptPos.y), projCol, 1.0f, 4.0f, 3.0f);
        }

        // Геометрическая форма маркера
        float s = theme.Scale(m.size);
        DrawMarkerShape(dl, ptPos, s, m.shape, markerColor, style.colBg);  // обводка — цветом холста

        // Текстовая метка (label / sublabel)
        if (!m.label.empty()) {
            theme.PushFont(theme.fontBold, 14.0f);
            ImVec2 lblPos(ptPos.x + s + theme.Scale(4.0f), ptPos.y - theme.Scale(14.0f));
            dl->AddText(lblPos, markerColor, m.label.c_str());
            theme.PopFont();

            if (!m.sublabel.empty()) {
                theme.PushFont(theme.fontRegular, 12.0f);
                dl->AddText(ImVec2(lblPos.x, lblPos.y + theme.Scale(14.0f)),
                            style.colAxisTitle, m.sublabel.c_str());
                theme.PopFont();
            }
        }
    }
}

void AnalysisChart::RenderCrosshairAndTooltip(ImDrawList* dl, const ChartCore::Projection& proj, const ChartStyle& style) {
    const UiTheme& theme = UiTheme::Get();
    ImVec2 mouse = ImGui::GetMousePos();

    // 1. Линии перекрестия
    dl->AddLine(ImVec2(mouse.x, proj.plotMin.y), ImVec2(mouse.x, proj.plotMax.y), style.colCrosshair, style.crosshairWidth);
    dl->AddLine(ImVec2(proj.plotMin.x, mouse.y), ImVec2(proj.plotMax.x, mouse.y), style.colCrosshair, style.crosshairWidth);

    // 2. Поиск ближайшей точки серии
    double mouseDataX = proj.ToDataX(mouse.x);
    double mouseDataY = proj.ToDataY(mouse.y);

    ChartHoverInfo info;
    info.hasHover = true;
    info.dataX = mouseDataX;
    info.dataY = mouseDataY;
    info.screenPos = mouse;

    if (!m_series.empty()) {
        int bestSeries = 0;
        int bestIdx = -1;
        double bestDist = 1e12;

        for (int sIdx = 0; sIdx < static_cast<int>(m_series.size()); ++sIdx) {
            const auto& s = m_series[static_cast<size_t>(sIdx)];
            if (s.points.empty()) continue;

            int idx = ChartCore::NearestIndexByX(s.points, mouseDataX);
            if (idx >= 0 && idx < static_cast<int>(s.points.size())) {
                const auto& pt = s.points[static_cast<size_t>(idx)];
                ImVec2 scr = proj.ToScreen(pt);
                double d = std::hypot(scr.x - mouse.x, scr.y - mouse.y);
                if (d < bestDist) {
                    bestDist = d;
                    bestSeries = sIdx;
                    bestIdx = idx;
                }
            }
        }

        if (bestIdx >= 0) {
            info.seriesIndex = bestSeries;
            info.pointIndex = static_cast<size_t>(bestIdx);
            const auto& bestPt = m_series[static_cast<size_t>(bestSeries)].points[static_cast<size_t>(bestIdx)];
            info.dataX = bestPt.x;
            info.dataY = bestPt.y;

            // Подсветка точки кружком
            ImVec2 hlPos = proj.ToScreen(bestPt);
            const ImU32 seriesColor = m_series[static_cast<size_t>(bestSeries)].color;
            dl->AddCircleFilled(hlPos, 4.0f, seriesColor ? seriesColor : style.colLine);
            dl->AddCircle(hlPos, 7.0f, style.colTooltipText, 16, 2.0f);
        }
    }

    m_hoverInfo = info;

    // 3. Тултип
    if (m_customTooltip) {
        m_customTooltip(info);
    } else {
        char buf[128];
        const char* xU = m_xUnit.empty() ? "" : (" " + m_xUnit).c_str();
        const char* yU = m_yUnit.empty() ? "" : (" " + m_yUnit).c_str();

        if (info.seriesIndex >= 0 && !m_series[static_cast<size_t>(info.seriesIndex)].label.empty()) {
            std::snprintf(buf, sizeof(buf), "%s\nX: %.2f%s\nY: %.2f%s",
                          m_series[static_cast<size_t>(info.seriesIndex)].label.c_str(),
                          info.dataX, xU, info.dataY, yU);
        } else {
            std::snprintf(buf, sizeof(buf), "X: %.2f%s\nY: %.2f%s",
                          info.dataX, xU, info.dataY, yU);
        }
        ImGui::SetTooltip("%s", buf);
    }
}
