#include "ImageViewerPage.hpp"
#include "omnikit.hpp"

#include <string>

namespace OmniKitShowcase {

namespace {

OmniKit::ImageViewer s_imageViewer;
bool s_imageLoaded = false;

void EnsureDemoImage() {
    if (!s_imageLoaded) {
        auto img = OmniKit::GenerateTestPattern(1024, 768);
        s_imageViewer.LoadImage16(img.data(), 1024, 768);
        s_imageLoaded = true;
    }
}

} // namespace

void RenderImageViewerPage() {
    EnsureDemoImage();
    const auto& theme = UiTheme::Get();
    s_imageViewer.SetStyle(OmniKit::MakeImageViewerStyle(theme));
    s_imageViewer.GetHistogram().SetStyle(OmniKit::MakeImageHistogramStyle(theme));

    SplitView split({.sideWidth = 340.0f, .key = "ImageViewerSplit"});

    if (auto main = split.Main({.cardBackground = true})) {
        if (ToolButton({.icon = Icon::Target, .tooltip = "Fit to View", .key = "fit_btn"})) {
            s_imageViewer.FitToView();
        }
        SameLine();
        if (ToolButton({.icon = Icon::Maximize, .tooltip = "100% Zoom", .key = "zoom100_btn"})) {
            s_imageViewer.ResetZoom();
        }
        SameLine();
        if (ToolButton({.icon = Icon::Plus, .tooltip = "Zoom In (+)", .key = "zoomin_btn"})) {
            s_imageViewer.SetZoom(s_imageViewer.GetZoom() * 1.25f);
        }
        SameLine();
        if (ToolButton({.glyph = "-", .tooltip = "Zoom Out (-)", .key = "zoomout_btn"})) {
            s_imageViewer.SetZoom(s_imageViewer.GetZoom() / 1.25f);
        }
        SameLine();
        if (ToolButton({.icon = Icon::RefreshCw, .tooltip = "Rotate CW 90°", .key = "rotcw_btn"})) {
            s_imageViewer.RotateCW();
        }
        SameLine();
        if (ToolButton({.icon = Icon::ArrowLeft, .tooltip = "Flip Horizontal",
                        .selected = s_imageViewer.GetFlipHorizontal(), .key = "fliph_btn"})) {
            s_imageViewer.ToggleFlipHorizontal();
        }
        SameLine();
        if (ToolButton({.icon = Icon::ArrowDown, .tooltip = "Flip Vertical",
                        .selected = s_imageViewer.GetFlipVertical(), .key = "flipv_btn"})) {
            s_imageViewer.ToggleFlipVertical();
        }
        SameLine();
        if (ToolButton({.icon = Icon::LineChart, .tooltip = "Profile Slice Tool (Hold RMB on image)",
                        .selected = s_imageViewer.IsProfileToolEnabled(), .key = "profile_btn"})) {
            s_imageViewer.SetProfileTool(!s_imageViewer.IsProfileToolEnabled());
        }
        SameLine();
        if (ToolButton({.icon = Icon::Refresh, .tooltip = "Reset to Default Test Pattern", .key = "reset_btn"})) {
            auto img = OmniKit::GenerateTestPattern(1024, 768);
            s_imageViewer.LoadImage16(img.data(), 1024, 768);
        }

        s_imageViewer.Render();
    }

    if (auto side = split.Side()) {
        auto& params = s_imageViewer.GetParamsMutable();

        // 1. Histogram & Auto-Fit Contrast Card
        if (Card card(CardOptions{.title = "Histogram & Contrast", .key = "CardHist"}); card) {
            OmniKit::WindowLevelParams newParams = params;
            if (auto col = card.Col(Col::Full())) {
                if (s_imageViewer.GetHistogram().Render(params, newParams, col.Width(), 85.0f)) {
                    s_imageViewer.SetParams(newParams);
                }
            }

            if (card.Button({.label = "Auto Window (Fit Contrast)", .variant = UiVariant::Primary, .col = Col::Full(), .key = "autowin_btn"})) {
                s_imageViewer.AutoFitWindow();
            }

            if (card.Button({.label = "Reset Window (Full Range)", .variant = UiVariant::Default, .col = Col::Full(), .key = "resetwin_btn"})) {
                float minV = s_imageViewer.GetHistogram().GetMinValue();
                float maxV = s_imageViewer.GetHistogram().GetMaxValue();
                params.window = (maxV > minV) ? (maxV - minV) : 65535.0f;
                params.level = (minV + maxV) * 0.5f;
                s_imageViewer.SetParams(params);
            }
        }

        // 2. Window / Level Controls Card
        if (Card card(CardOptions{.title = "Window & Level Controls", .key = "CardWL"}); card) {
            if (card.Float(params.window, {.label = "Window Width (W)", .format = "%.0f", .col = Col::Full()})) {
                if (params.window < 1.0f) params.window = 1.0f;
                s_imageViewer.SetParams(params);
            }

            if (card.Float(params.level, {.label = "Window Level (L)", .format = "%.0f", .col = Col::Full()})) {
                s_imageViewer.SetParams(params);
            }

            if (card.Float(params.gamma, {.label = "Gamma (Midtones)", .format = "%.2f", .col = Col::Full()})) {
                if (params.gamma < 0.1f) params.gamma = 0.1f;
                if (params.gamma > 5.0f) params.gamma = 5.0f;
                s_imageViewer.SetParams(params);
            }

            bool inv = params.invert;
            if (card.Toggle(inv, {.label = "Invert Polarity (MONOCHROME1)", .col = Col::Full(), .key = "toggle_invert"})) {
                params.invert = inv;
                s_imageViewer.SetParams(params);
            }
        }

        // 3. Colormap Card
        if (Card card(CardOptions{.title = "Colormap", .key = "CardColormap"}); card) {
            bool isGray = (params.colormap == OmniKit::ImageColormap::Grayscale);
            if (card.Button({.label = "Grayscale", .variant = isGray ? UiVariant::Primary : UiVariant::Default, .col = Col::Half(), .key = "cmap_gray"})) {
                params.colormap = OmniKit::ImageColormap::Grayscale;
                s_imageViewer.SetParams(params);
            }

            bool isTurbo = (params.colormap == OmniKit::ImageColormap::Turbo);
            if (card.Button({.label = "Turbo (HDR)", .variant = isTurbo ? UiVariant::Primary : UiVariant::Default, .col = Col::Half(), .key = "cmap_turbo"})) {
                params.colormap = OmniKit::ImageColormap::Turbo;
                s_imageViewer.SetParams(params);
            }
        }

        // 4. Pixel Inspector & Metrics Card
        if (Card card(CardOptions{.title = "Pixel Inspector", .key = "CardInspector"}); card) {
            auto hover = s_imageViewer.GetHoverInfo();
            if (hover.valid) {
                card.Value(static_cast<double>(hover.pixelX), {.label = "Hovered X", .format = "%.0f", .col = Col::Half()});
                card.Value(static_cast<double>(hover.pixelY), {.label = "Hovered Y", .format = "%.0f", .col = Col::Half()});
                card.Value(static_cast<double>(hover.rawValue), {.label = "Raw 16-bit Intensity", .unit = "/ 65535", .format = "%.0f", .col = Col::Full()});
            } else {
                card.Value("Hover over image", {.label = "Cursor", .col = Col::Full()});
            }

            std::string dimStr = std::to_string(s_imageViewer.GetWidth()) + " × " + std::to_string(s_imageViewer.GetHeight()) + " px";
            card.Value(dimStr.c_str(), {.label = "Image Dimensions", .unit = "16-bit HDR", .col = Col::Full()});

            card.Value(static_cast<double>(s_imageViewer.GetZoom() * 100.0f), {.label = "Zoom Level", .unit = "%", .format = "%.0f", .col = Col::Half()});

            card.Value(static_cast<double>(s_imageViewer.GetRotation()), {.label = "Rotation", .unit = "\xC2\xB0", .format = "%.0f", .col = Col::Half()});
        }
    }
}

} // namespace OmniKitShowcase
