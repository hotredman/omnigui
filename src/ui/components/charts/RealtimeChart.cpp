#include "ui/components/charts/RealtimeChart.hpp"
#include "ui/components/UiTheme.hpp"
#include "core/units/Units.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

RealtimeChart::RealtimeChart() {
    Clear();
    UpdateAxisLabels();
}

const ChartStyle& RealtimeChart::GetStyle() const {
    if (m_hasCustomStyle) {
        return m_style;
    }
    return UiTheme::Get().chart;
}

void RealtimeChart::SetAxisPair(RealtimeAxisPair pair) {
    if (m_axisPair == pair) return;
    m_axisPair = pair;
    UpdateAxisLabels();
    RebuildPointsFromSamples();
}

void RealtimeChart::UpdateAxisLabels() {
    switch (m_axisPair) {
        case RealtimeAxisPair::Force_Displacement:
            m_xLabel = "Displacement ΔL";
            m_xUnit  = Units::Label(PhysicalUnit::Mm);
            m_yLabel = "Load F";
            m_yUnit  = Units::Label(PhysicalUnit::KN);
            break;
        case RealtimeAxisPair::Force_Time:
            m_xLabel = "Elapsed Time t";
            m_xUnit  = Units::Label(PhysicalUnit::S);
            m_yLabel = "Load F";
            m_yUnit  = Units::Label(PhysicalUnit::KN);
            break;
        case RealtimeAxisPair::Displacement_Time:
            m_xLabel = "Elapsed Time t";
            m_xUnit  = Units::Label(PhysicalUnit::S);
            m_yLabel = "Displacement ΔL";
            m_yUnit  = Units::Label(PhysicalUnit::Mm);
            break;
        case RealtimeAxisPair::Force_Extensometer:
            m_xLabel = "Extensometer ΔL_ext";
            m_xUnit  = Units::Label(PhysicalUnit::Mm);
            m_yLabel = "Load F";
            m_yUnit  = Units::Label(PhysicalUnit::KN);
            break;
    }
}

void RealtimeChart::SetXAxis(std::string label, std::string unit) {
    m_xLabel = std::move(label);
    m_xUnit  = std::move(unit);
}

void RealtimeChart::SetYAxis(std::string label, std::string unit) {
    m_yLabel = std::move(label);
    m_yUnit  = std::move(unit);
}

void RealtimeChart::Clear() {
    m_samples.clear();
    m_points.clear();
    m_screenPoints.clear();
    m_rawMaxX = 0.0;
    m_rawMaxY = 0.0;

    m_bounds.minX = 0.0;
    m_bounds.maxX = 10.0;
    m_bounds.minY = 0.0;
    m_bounds.maxY = 10.0;
}

std::pair<double, double> RealtimeChart::ExtractPoint(const RealtimeSample& s) const {
    switch (m_axisPair) {
        case RealtimeAxisPair::Force_Displacement:
            return { s.displacementMm, s.forceKN };
        case RealtimeAxisPair::Force_Time:
            return { s.timeS, s.forceKN };
        case RealtimeAxisPair::Displacement_Time:
            return { s.timeS, s.displacementMm };
        case RealtimeAxisPair::Force_Extensometer:
            return { s.extensometerMm, s.forceKN };
        default:
            return { s.displacementMm, s.forceKN };
    }
}

void RealtimeChart::AppendSample(double timeS, double forceKN, double displacementMm, double extensometerMm) {
    AppendSample(RealtimeSample(timeS, forceKN, displacementMm, extensometerMm));
}

void RealtimeChart::AppendSample(const RealtimeSample& sample) {
    m_samples.push_back(sample);
    auto [x, y] = ExtractPoint(sample);
    AppendPoint(x, y);
}

void RealtimeChart::AppendPoint(double x, double y) {
    m_points.emplace_back(x, y);
    UpdateBoundsWithPoint(x, y);
}

void RealtimeChart::UpdateBoundsWithPoint(double x, double y) {
    if (x < m_bounds.minX) m_bounds.minX = x;
    if (y < m_bounds.minY) m_bounds.minY = y;

    m_rawMaxX = std::max(m_rawMaxX, x);
    m_rawMaxY = std::max(m_rawMaxY, y);

    // Автомасштаб с запасом +10% сверху и справа
    m_bounds.maxX = std::max(1.0, m_rawMaxX * 1.10);
    m_bounds.maxY = std::max(1.0, m_rawMaxY * 1.10);
}

void RealtimeChart::RebuildPointsFromSamples() {
    m_points.clear();
    m_screenPoints.clear();
    m_rawMaxX = 0.0;
    m_rawMaxY = 0.0;
    m_bounds.minX = 0.0;
    m_bounds.maxX = 10.0;
    m_bounds.minY = 0.0;
    m_bounds.maxY = 10.0;

    m_points.reserve(m_samples.size());
    for (const auto& s : m_samples) {
        auto [x, y] = ExtractPoint(s);
        m_points.emplace_back(x, y);
        UpdateBoundsWithPoint(x, y);
    }
}

void RealtimeChart::Render(ImVec2 size) {
    ImGui::PushID(this);

    const UiTheme& theme = UiTheme::Get();
    ImVec2 canvasSize = size;
    if (canvasSize.x <= 0.0f) canvasSize.x = ImGui::GetContentRegionAvail().x;
    if (canvasSize.y <= 0.0f) canvasSize.y = ImGui::GetContentRegionAvail().y;

    if (canvasSize.x < 100.0f) canvasSize.x = 100.0f;
    if (canvasSize.y < 80.0f) canvasSize.y = 80.0f;

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // 1. Холст графика и внешняя граница
    const ChartStyle& style = GetStyle();
    float radius = theme.Scale(style.cornerRadius);
    dl->AddRectFilled(origin, ImVec2(origin.x + canvasSize.x, origin.y + canvasSize.y), style.colBg, radius);
    dl->AddRect(origin, ImVec2(origin.x + canvasSize.x, origin.y + canvasSize.y), style.colBorder, radius, 0, style.borderSize);

    // 2. Создание проекции координат
    ChartPad scaledPad = m_pad;
    scaledPad.left = theme.Scale(m_pad.left);
    scaledPad.right = theme.Scale(m_pad.right);
    scaledPad.top = theme.Scale(m_pad.top);
    scaledPad.bottom = theme.Scale(m_pad.bottom);

    ChartCore::Projection proj = ChartCore::Projection::Create(m_bounds, scaledPad, origin, canvasSize);

    // 3. Сетка и тики
    std::vector<double> ticksX = ChartCore::GenerateTicks(m_bounds.minX, m_bounds.maxX, 5);
    std::vector<double> ticksY = ChartCore::GenerateTicks(m_bounds.minY, m_bounds.maxY, 5);

    const ImU32 colGrid = style.colMajorGrid;
    const ImU32 colText = style.colTickText;

    theme.PushFont(theme.fontRegular, style.tickFontSize);

    // Вертикальные линии сетки и подписи X
    for (double tx : ticksX) {
        float sx = proj.ToScreenX(tx);
        if (sx >= proj.plotMin.x - 1.0f && sx <= proj.plotMax.x + 1.0f) {
            dl->AddLine(ImVec2(sx, proj.plotMin.y), ImVec2(sx, proj.plotMax.y), colGrid, style.majorGridWidth);

            std::string label = ChartCore::FormatTickValue(tx);
            ImVec2 txtSz = ImGui::CalcTextSize(label.c_str());
            dl->AddText(ImVec2(sx - txtSz.x * 0.5f, proj.plotMax.y + theme.Scale(6.0f)), colText, label.c_str());
        }
    }

    // Горизонтальные линии сетки и подписи Y
    for (double ty : ticksY) {
        float sy = proj.ToScreenY(ty);
        if (sy >= proj.plotMin.y - 1.0f && sy <= proj.plotMax.y + 1.0f) {
            dl->AddLine(ImVec2(proj.plotMin.x, sy), ImVec2(proj.plotMax.x, sy), colGrid, style.majorGridWidth);

            std::string label = ChartCore::FormatTickValue(ty);
            ImVec2 txtSz = ImGui::CalcTextSize(label.c_str());
            dl->AddText(ImVec2(proj.plotMin.x - txtSz.x - theme.Scale(6.0f), sy - txtSz.y * 0.5f), colText, label.c_str());
        }
    }

    // 4. Внутренняя рамка области графика
    dl->AddRect(proj.plotMin, proj.plotMax, style.colAxis, 0.0f, 0, 1.0f);

    theme.PopFont();

    // 5. Заголовки осей
    theme.PushFont(theme.fontMedium, style.axisTitleFontSize);
    const ImU32 colTitle = style.colAxisTitle;

    // Заголовок оси Y (сверху слева)
    if (!m_yLabel.empty()) {
        std::string yTitle = m_yLabel + (m_yUnit.empty() ? "" : (" (" + m_yUnit + ")"));
        float yPos = (proj.plotMin.y - origin.y >= theme.Scale(24.0f))
                         ? (origin.y + theme.Scale(3.0f))
                         : (proj.plotMin.y + theme.Scale(4.0f));
        dl->AddText(ImVec2(proj.plotMin.x + theme.Scale(6.0f), yPos), colTitle, yTitle.c_str());
    }

    // Заголовок оси X (снизу справа вдоль графика)
    if (!m_xLabel.empty()) {
        std::string xTitle = m_xLabel + (m_xUnit.empty() ? "" : (" (" + m_xUnit + ")"));
        ImVec2 xTitleSz = ImGui::CalcTextSize(xTitle.c_str());
        dl->AddText(ImVec2(proj.plotMax.x - xTitleSz.x, proj.plotMax.y + theme.Scale(28.0f)), colTitle, xTitle.c_str());
    }

    theme.PopFont();

    // 6. Отрисовка кривой с отсечением по области графика
    dl->PushClipRect(proj.plotMin, proj.plotMax, true);

    if (m_points.empty()) {
        const char* emptyMsg = "Waiting for telemetry stream...";
        ImVec2 msgSz = ImGui::CalcTextSize(emptyMsg);
        ImVec2 msgPos(proj.plotMin.x + (proj.plotWidth - msgSz.x) * 0.5f,
                      proj.plotMin.y + (proj.plotHeight - msgSz.y) * 0.5f);
        dl->AddText(msgPos, colText, emptyMsg);
    } else {
        m_screenPoints.resize(m_points.size());
        for (size_t i = 0; i < m_points.size(); ++i) {
            m_screenPoints[i] = proj.ToScreen(m_points[i]);
        }

        const ImU32 lineColor = m_lineColor ? m_lineColor : style.colLine;
        dl->AddPolyline(m_screenPoints.data(), static_cast<int>(m_screenPoints.size()), lineColor, 0, theme.Scale(m_lineThickness));

        // Яркая точка-маркер на конце кривой (текущая точка испытания)
        if (m_showHeadMarker) {
            ImVec2 headPos = m_screenPoints.back();
            dl->AddCircleFilled(headPos, theme.Scale(6.0f), Tone::Alpha(lineColor, 0.28f));
            dl->AddCircleFilled(headPos, theme.Scale(3.5f), theme.palette.textOnAccent);
        }
    }

    dl->PopClipRect();

    // Резервируем пространство в компоновщике ImGui
    ImGui::Dummy(canvasSize);

    ImGui::PopID();
}
