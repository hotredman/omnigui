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
        OmniKit::ImageViewerToolbar(s_imageViewer, {
            .onReset = [&] {
                auto img = OmniKit::GenerateTestPattern(1024, 768);
                s_imageViewer.LoadImage16(img.data(), 1024, 768);
            }
        });
        s_imageViewer.Render();
        OmniKit::ImageViewerContextMenu(s_imageViewer, "##image_viewer_ctx", [&](OmniKit::ContextMenu& menu) {
            menu.Header("CONTRAST");
            if (menu.Item("Auto Fit Contrast")) {
                s_imageViewer.AutoFitWindow();
            }
        });
    }

    if (auto side = split.Side()) {
        auto& params = s_imageViewer.GetParamsMutable();

        // 1. Histogram & Auto-Fit Contrast Card
        OmniKit::ImageViewerHistogramCard(s_imageViewer);

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

            char dimBuf[64];
            std::snprintf(dimBuf, sizeof(dimBuf), "%d \xC3\x97 %d px", s_imageViewer.GetWidth(), s_imageViewer.GetHeight());
            card.Value(dimBuf, {.label = "Image Dimensions", .unit = "16-bit HDR", .col = Col::Full()});

            card.Value(static_cast<double>(s_imageViewer.GetZoom() * 100.0f), {.label = "Zoom Level", .unit = "%", .format = "%.0f", .col = Col::Half()});

            card.Value(static_cast<double>(s_imageViewer.GetRotation()), {.label = "Rotation", .unit = "\xC2\xB0", .format = "%.0f", .col = Col::Half()});
        }
    }
}

} // namespace OmniKitShowcase
