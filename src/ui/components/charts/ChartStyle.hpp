#pragma once

#include <imgui.h>
#include <algorithm>

// ============================================================================
// Style descriptor for charts (ChartStyle)
// Fully autonomous from UiTheme, depends only on Dear ImGui and C++20.
// ============================================================================
struct ChartStyle {
    // Canvas background and border colors
    ImU32 colBg             = IM_COL32(15, 23, 42, 255);   // Slate900
    ImU32 colBorder         = IM_COL32(51, 65, 85, 255);   // Slate700
    float cornerRadius      = 6.0f;
    float borderSize        = 1.0f;

    // Default curve (series / line / marker without custom color)
    ImU32 colLine           = IM_COL32(56, 189, 248, 255);  // Sky400

    // Grid
    ImU32 colMajorGrid      = IM_COL32(51, 65, 85, 128);   // Slate700 subtle
    ImU32 colMinorGrid      = IM_COL32(30, 41, 59, 100);   // Slate800 subtle
    float majorGridWidth    = 1.0f;
    float minorGridWidth    = 1.0f;

    // Axes and tick labels
    ImU32 colAxis           = IM_COL32(71, 85, 105, 255);  // Slate600
    ImU32 colTickText       = IM_COL32(148, 163, 184, 255); // Slate400
    ImU32 colAxisTitle      = IM_COL32(203, 213, 225, 255); // Slate300
    float tickFontSize      = 18.0f;
    float axisTitleFontSize = 20.0f;

    // Probe / Crosshair
    ImU32 colCrosshair      = IM_COL32(248, 250, 252, 180); // White alpha
    float crosshairWidth    = 1.0f;

    // Marker projections
    ImU32 colProjection     = IM_COL32(148, 163, 184, 150);

    // Tooltip
    ImU32 colTooltipBg      = IM_COL32(15, 23, 42, 240);
    ImU32 colTooltipBorder  = IM_COL32(56, 189, 248, 255);
    ImU32 colTooltipText    = IM_COL32(248, 250, 252, 255);

    // Accent and text on accent
    ImU32 colTextOnAccent   = IM_COL32(255, 255, 255, 255);

    // Scale and typography
    float scale             = 1.0f;
    ImFont* fontRegular     = nullptr;
    ImFont* fontMedium      = nullptr;
    ImFont* fontBold        = nullptr;

    float Scale(float v) const { return v * scale; }

    static ImU32 WithAlpha(ImU32 col, float alpha) {
        ImVec4 c = ImGui::ColorConvertU32ToFloat4(col);
        c.w *= std::clamp(alpha, 0.0f, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(c);
    }

    void PushFont(ImFont* font, float basePt) const {
        ImFont* f = font ? font : (fontRegular ? fontRegular : ImGui::GetFont());
        ImGui::PushFont(f, Scale(basePt));
    }

    void PopFont() const {
        ImGui::PopFont();
    }

    static ChartStyle Dark() {
        return ChartStyle{};
    }

    static ChartStyle Light() {
        ChartStyle s;
        s.colBg             = IM_COL32(255, 255, 255, 255); // White
        s.colBorder         = IM_COL32(226, 232, 240, 255); // Slate200
        s.colLine           = IM_COL32(2, 132, 199, 255);   // Sky600
        s.colMajorGrid      = IM_COL32(226, 232, 240, 200); // Slate200
        s.colMinorGrid      = IM_COL32(241, 245, 249, 180); // Slate100
        s.colAxis           = IM_COL32(203, 213, 225, 255); // Slate300
        s.colTickText       = IM_COL32(100, 116, 139, 255); // Slate500
        s.colAxisTitle      = IM_COL32(51, 65, 85, 255);   // Slate700
        s.colCrosshair      = IM_COL32(15, 23, 42, 180);    // Slate900 alpha
        s.colProjection     = IM_COL32(148, 163, 184, 180);
        s.colTooltipBg      = IM_COL32(255, 255, 255, 245);
        s.colTooltipBorder  = IM_COL32(2, 132, 199, 255);
        s.colTooltipText    = IM_COL32(15, 23, 42, 255);
        s.colTextOnAccent   = IM_COL32(255, 255, 255, 255);
        return s;
    }
};
