#include "omnikit/ImageViewerControls.hpp"
#include "omnikit/ContextMenu.hpp"
#include "omnikit/ToolButton.hpp"
#include "omnikit/Card.hpp"
#include "omnikit/Icon.hpp"
#include "omnikit/Layout.hpp"
#include "omnikit/ColumnLayout.hpp"
#include "omnikit/IdScope.hpp"
#include <imgui.h>

namespace OmniKit {

void ImageViewerToolbar(ImageViewer& viewer, const ImageViewerToolbarOptions& options) {
    IdScope id(options.keyPrefix ? options.keyPrefix : "iv_tb");

    bool first = true;
    auto addSpacer = [&]() {
        if (!first) {
            SameLine();
        }
        first = false;
    };

    if (options.showFit) {
        addSpacer();
        if (ToolButton({.icon = Icon::Target, .tooltip = "Fit to View", .key = "fit"})) {
            viewer.FitToView();
        }
    }

    if (options.showZoomReset) {
        addSpacer();
        if (ToolButton({.icon = Icon::Maximize, .tooltip = "100% Zoom", .key = "z100"})) {
            viewer.ResetZoom();
        }
    }

    if (options.showZoomControls) {
        addSpacer();
        if (ToolButton({.icon = Icon::Plus, .tooltip = "Zoom In (+)", .key = "zin"})) {
            viewer.SetZoom(viewer.GetZoom() * options.zoomFactor);
        }
        SameLine();
        if (ToolButton({.icon = Icon::Minus, .tooltip = "Zoom Out (-)", .key = "zout"})) {
            viewer.SetZoom(viewer.GetZoom() / options.zoomFactor);
        }
    }

    if (options.showRotate) {
        addSpacer();
        if (ToolButton({.icon = Icon::RefreshCw, .tooltip = "Rotate CW 90°", .key = "rotcw"})) {
            viewer.RotateCW();
        }
    }

    if (options.showFlip) {
        addSpacer();
        if (ToolButton({.icon = Icon::ArrowLeft, .tooltip = "Flip Horizontal",
                        .selected = viewer.GetFlipHorizontal(), .key = "fliph"})) {
            viewer.ToggleFlipHorizontal();
        }
        SameLine();
        if (ToolButton({.icon = Icon::ArrowDown, .tooltip = "Flip Vertical",
                        .selected = viewer.GetFlipVertical(), .key = "flipv"})) {
            viewer.ToggleFlipVertical();
        }
    }

    if (options.showProfileTool) {
        addSpacer();
        if (ToolButton({.icon = Icon::LineChart, .tooltip = "Profile Slice Tool (Hold RMB on image)",
                        .selected = viewer.IsProfileToolEnabled(), .key = "prof"})) {
            viewer.SetProfileTool(!viewer.IsProfileToolEnabled());
        }
    }

    if (options.showReset) {
        addSpacer();
        if (ToolButton({.icon = Icon::Refresh, .tooltip = "Reset", .key = "rst"})) {
            if (options.onReset) {
                options.onReset();
            }
        }
    }
}

static void PopulateImageViewerContextMenu(ContextMenu& menu, ImageViewer& viewer, const std::function<void(ContextMenu&)>& extraItems) {
    menu.Header("IMAGE VIEW");
    if (menu.Item("Fit to View")) {
        viewer.FitToView();
    }
    if (menu.Item("Reset Zoom (100%)")) {
        viewer.ResetZoom();
    }

    menu.Separator();
    menu.Header("ZOOM PRESETS");
    if (menu.Item("Zoom 50%"))  viewer.SetZoom(0.5f);
    if (menu.Item("Zoom 200%")) viewer.SetZoom(2.0f);
    if (menu.Item("Zoom 400%")) viewer.SetZoom(4.0f);

    menu.Separator();
    menu.Header("ORIENTATION");
    if (menu.Item("Rotate CW 90\xC2\xB0"))  viewer.RotateCW();
    if (menu.Item("Rotate CCW 90\xC2\xB0")) viewer.RotateCCW();
    if (menu.Item("Flip Horizontal", {.selected = viewer.GetFlipHorizontal()})) {
        viewer.ToggleFlipHorizontal();
    }
    if (menu.Item("Flip Vertical", {.selected = viewer.GetFlipVertical()})) {
        viewer.ToggleFlipVertical();
    }

    if (extraItems) {
        menu.Separator();
        extraItems(menu);
    }
}

bool ImageViewerContextMenu(ImageViewer& viewer, const char* popupId, std::function<void(ContextMenu&)> extraItems) {
    if (ContextMenu menu(popupId ? popupId : "##image_viewer_ctx"); menu) {
        PopulateImageViewerContextMenu(menu, viewer, extraItems);
        return true;
    }
    return false;
}

bool ImageViewerContextMenu(ImageViewer& viewer, bool& isOpen, std::function<void(ContextMenu&)> extraItems) {
    if (ContextMenu menu(isOpen); menu) {
        PopulateImageViewerContextMenu(menu, viewer, extraItems);
        return true;
    }
    return false;
}

void ImageViewerHistogramCard(ImageViewer& viewer, const ImageViewerHistogramOptions& options) {
    auto& params = viewer.GetParamsMutable();
    const char* cardTitle = options.title ? options.title : "Histogram & Contrast";
    const char* cardKey = options.key ? options.key : "CardHist";

    if (Card card(CardOptions{.title = cardTitle, .key = cardKey}); card) {
        WindowLevelParams newParams = params;
        if (auto col = card.Col(Col::Full())) {
            if (viewer.GetHistogram().Render(params, newParams, col.Width(), options.height)) {
                viewer.SetParams(newParams);
            }
        }

        const bool bothButtons = options.showAutoWindow && options.showResetWindow;
        const Col btnCol = bothButtons ? Col::Half() : Col::Full();

        if (options.showAutoWindow) {
            const char* label = options.autoWindowLabel ? options.autoWindowLabel : "Auto Window";
            if (card.Button({.label = label, .variant = UiVariant::Primary,
                             .tooltip = "Auto Window (Fit Contrast)", .col = btnCol, .key = "autowin"})) {
                viewer.AutoFitWindow();
            }
        }

        if (options.showResetWindow) {
            const char* label = options.resetWindowLabel ? options.resetWindowLabel : "Reset Window";
            if (card.Button({.label = label, .variant = UiVariant::Default,
                             .tooltip = "Reset Window (Full Range)", .col = btnCol, .key = "rstwin"})) {
                float minV = viewer.GetHistogram().GetMinValue();
                float maxV = viewer.GetHistogram().GetMaxValue();
                WindowLevelParams resetParams = viewer.GetParams();
                resetParams.window = (maxV > minV) ? (maxV - minV) : 65535.0f;
                resetParams.level = (minV + maxV) * 0.5f;
                viewer.SetParams(resetParams);
            }
        }
    }
}

} // namespace OmniKit
