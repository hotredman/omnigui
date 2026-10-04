#pragma once

#include "ui/components/charts/ChartStyle.hpp"
#include "ui/components/UiTheme.hpp"

namespace OmniKit {

// ============================================================================
// Адаптер дизайн-системы для подсистемы Charts
// Чистое связывание (Adapter Pattern): ни UiTheme, ни графики не знают
// друг о друге, конвертация токенов происходит здесь.
// ============================================================================
inline ChartStyle MakeChartStyle(const UiTheme& theme) {
    const auto& p = theme.palette;
    ChartStyle s;
    s.colBg             = p.chartBg;
    s.colBorder         = p.border;
    s.cornerRadius      = theme.CornerRadius();
    s.borderSize        = 1.0f;
    s.colLine           = p.chartLine;
    s.colMajorGrid      = p.chartGridMajor;
    s.colMinorGrid      = p.chartGridMinor;
    s.majorGridWidth    = 1.0f;
    s.minorGridWidth    = 1.0f;
    s.colAxis           = p.chartAxis;
    s.colTickText       = p.chartText;
    s.colAxisTitle      = p.chartText;
    s.tickFontSize      = 18.0f;
    s.axisTitleFontSize = 20.0f;
    s.colCrosshair      = p.chartCrosshair;
    s.crosshairWidth    = 1.0f;
    s.colProjection     = p.chartProjection;
    s.colTooltipBg      = p.tooltipBg;
    s.colTooltipBorder  = p.accentBorder;
    s.colTooltipText    = p.tooltipText;
    s.colTextOnAccent   = p.textOnAccent;

    s.scale             = theme.GetScale();
    s.fontRegular       = theme.fontRegular;
    s.fontMedium        = theme.fontMedium;
    s.fontBold          = theme.fontBold;

    return s;
}

} // namespace OmniKit
