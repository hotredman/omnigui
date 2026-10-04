#pragma once

#include <imgui.h>

namespace OmniKit {

// ============================================================================
// Style descriptor for 16-bit Image Viewer (ImageViewerStyle)
// Fully autonomous from UiTheme, depends only on Dear ImGui and C++20.
// ============================================================================
struct ImageViewerStyle {
    ImU32 colBg                 = IM_COL32(15, 23, 42, 255);   // Canvas background (Slate900 / Slate100)
    ImU32 colBorder             = IM_COL32(51, 65, 85, 255);   // Canvas border
    ImU32 colImageBorder        = IM_COL32(100, 116, 139, 76); // Subtle frame outline
    ImU32 colSliceLine          = IM_COL32(239, 68, 68, 240);  // Slice tool line (Red500)
    ImU32 colSliceHandle        = IM_COL32(239, 68, 68, 255);  // Slice handle points
    ImU32 colOverlayBg          = IM_COL32(30, 41, 59, 216);   // Semi-transparent overlay background
    ImU32 colOverlayBorder      = IM_COL32(100, 116, 139, 76); // Overlay border
    ImU32 colOverlayText        = IM_COL32(241, 245, 249, 255); // Primary overlay text (Slate100)
    ImU32 colOverlaySecondary   = IM_COL32(148, 163, 184, 255); // Secondary overlay text (Slate400)
    ImU32 colProfileLine        = IM_COL32(56, 189, 248, 255);  // Slice profile line (Sky400)
    ImU32 colDivider            = IM_COL32(51, 65, 85, 255);   // Profile overlay divider
    float cornerRadius          = 8.0f;
    float scale                 = 1.0f;

    float Scale(float v) const { return v * scale; }

    static ImageViewerStyle Dark() {
        return ImageViewerStyle{};
    }

    static ImageViewerStyle Light() {
        ImageViewerStyle s;
        s.colBg                 = IM_COL32(241, 245, 249, 255); // Slate100
        s.colBorder             = IM_COL32(203, 213, 225, 255); // Slate300
        s.colImageBorder        = IM_COL32(148, 163, 184, 100); // Slate400 subtle
        s.colSliceLine          = IM_COL32(220, 38, 38, 240);   // Red600
        s.colSliceHandle        = IM_COL32(220, 38, 38, 255);
        s.colOverlayBg          = IM_COL32(255, 255, 255, 225); // White pill
        s.colOverlayBorder      = IM_COL32(203, 213, 225, 200);
        s.colOverlayText        = IM_COL32(15, 23, 42, 255);    // Slate900
        s.colOverlaySecondary   = IM_COL32(71, 85, 105, 255);   // Slate600
        s.colProfileLine        = IM_COL32(2, 132, 199, 255);   // Sky600
        s.colDivider            = IM_COL32(226, 232, 240, 255); // Slate200
        return s;
    }
};

// ============================================================================
// Style descriptor for interactive histogram (ImageHistogramStyle)
// ============================================================================
struct ImageHistogramStyle {
    ImU32 colBg                 = IM_COL32(15, 23, 42, 255);   // Slate900
    ImU32 colBorder             = IM_COL32(51, 65, 85, 255);   // Slate700
    ImU32 colBins               = IM_COL32(148, 163, 184, 178);// Slate400 alpha
    ImU32 colBand               = IM_COL32(56, 189, 248, 36);  // Sky400 alpha
    ImU32 colLines              = IM_COL32(56, 189, 248, 230); // Sky400
    ImU32 colHandles            = IM_COL32(56, 189, 248, 255); // Sky400
    float cornerRadius          = 6.0f;
    float scale                 = 1.0f;

    float Scale(float v) const { return v * scale; }

    static ImageHistogramStyle Dark() {
        return ImageHistogramStyle{};
    }

    static ImageHistogramStyle Light() {
        ImageHistogramStyle s;
        s.colBg                 = IM_COL32(255, 255, 255, 255); // White inset
        s.colBorder             = IM_COL32(203, 213, 225, 255); // Slate300
        s.colBins               = IM_COL32(71, 85, 105, 178);   // Slate600
        s.colBand               = IM_COL32(2, 132, 199, 25);    // Sky600 alpha
        s.colLines              = IM_COL32(2, 132, 199, 230);   // Sky600
        s.colHandles            = IM_COL32(2, 132, 199, 255);   // Sky600
        return s;
    }
};

} // namespace OmniKit
