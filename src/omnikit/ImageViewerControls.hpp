#pragma once

#include "omnikit/image/ImageViewer.hpp"
#include <functional>
#include <string>

namespace OmniKit {

class ContextMenu;

// ============================================================================
// Options for standard ImageViewer toolbar
// ============================================================================
struct ImageViewerToolbarOptions {
    bool showFit           = true;   // Fit to view button
    bool showZoomReset     = true;   // 100% zoom button
    bool showZoomControls  = true;   // Zoom In / Zoom Out buttons
    bool showRotate        = true;   // Rotate CW 90° button
    bool showFlip          = true;   // Flip Horizontal / Vertical toggle buttons
    bool showProfileTool   = true;   // Profile slice tool toggle button
    bool showReset         = true;   // Reset button
    float zoomFactor       = 1.25f;  // Step multiplier for zoom in / out
    const char* keyPrefix  = "iv_tb";
    std::function<void()> onReset = nullptr; // Optional custom reset callback
};

// ============================================================================
// Options for standard ImageViewer histogram & contrast card
// ============================================================================
struct ImageViewerHistogramOptions {
    const char* title            = "Histogram & Contrast";
    const char* key              = "CardHist";
    float height                 = 85.0f;
    bool showAutoWindow          = true;
    bool showResetWindow         = true;
    const char* autoWindowLabel  = nullptr;
    const char* resetWindowLabel = nullptr;
};

// ============================================================================
// High-Level OmniKit UI Controls for ImageViewer
// ============================================================================

// Renders the standard image navigation toolbar (ToolButton row)
void ImageViewerToolbar(ImageViewer& viewer, const ImageViewerToolbarOptions& options = {});

// Renders the standard context menu on right-click (Fit, Zoom presets, Rotate, Flip).
// Uses OmniKit::ContextMenu with design-system styling, section headers, and checkmarks.
// Optional extraItems callback allows host views to append custom domain actions.
bool ImageViewerContextMenu(ImageViewer& viewer,
                           const char* popupId = "##image_viewer_ctx",
                           std::function<void(ContextMenu&)> extraItems = nullptr);

// Overload for explicit open state flag (e.g. triggered by a tool button or custom action)
bool ImageViewerContextMenu(ImageViewer& viewer,
                           bool& isOpen,
                           std::function<void(ContextMenu&)> extraItems = nullptr);

// Renders the standard contrast & histogram card for sidebar/dock panels
void ImageViewerHistogramCard(ImageViewer& viewer, const ImageViewerHistogramOptions& options = {});

} // namespace OmniKit
