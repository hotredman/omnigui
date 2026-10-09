#include "ImageViewerPage.hpp"
#include "omnikit.hpp"

#include <string>

namespace OmniKitShowcase {

using namespace OmniKit;

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
        // 1. Histogram & Auto-Fit Contrast Card
        OmniKit::ImageViewerHistogramCard(s_imageViewer);

        // 2. Colormap Card
        if (Card card(CardOptions{.title = "Colormap", .key = "CardColormap"}); card) {
            bool isGray = (s_imageViewer.GetColormap() == OmniKit::ImageColormap::Grayscale);
            if (card.Button({.label = "Grayscale", .variant = isGray ? UiVariant::Primary : UiVariant::Default, .col = Col::Half(), .key = "cmap_gray"})) {
                s_imageViewer.SetColormap(OmniKit::ImageColormap::Grayscale);
            }

            bool isTurbo = (s_imageViewer.GetColormap() == OmniKit::ImageColormap::Turbo);
            if (card.Button({.label = "Turbo (HDR)", .variant = isTurbo ? UiVariant::Primary : UiVariant::Default, .col = Col::Half(), .key = "cmap_turbo"})) {
                s_imageViewer.SetColormap(OmniKit::ImageColormap::Turbo);
            }
        }

        // 3. Pixel Inspector & Metrics Card
        if (Card card(CardOptions{.title = "Pixel Inspector", .key = "CardInspector"}); card) {
            auto hover = s_imageViewer.GetHoverInfo();
            if (hover.valid) {
                card.Value(static_cast<double>(hover.pixelX), {.label = "Hovered X", .format = "%.0f", .col = Col::Half(), .size = UiSize::Small});
                card.Value(static_cast<double>(hover.pixelY), {.label = "Hovered Y", .format = "%.0f", .col = Col::Half(), .size = UiSize::Small});
                card.Value(static_cast<double>(hover.rawValue), {.label = "Raw 16-bit Intensity", .unit = "/ 65535", .format = "%.0f", .col = Col::Full(), .size = UiSize::Small});
            } else {
                card.Value("Hover over image", {.label = "Cursor", .col = Col::Full(), .size = UiSize::Small});
            }

            char dimBuf[64];
            std::snprintf(dimBuf, sizeof(dimBuf), "%d \xC3\x97 %d px", s_imageViewer.GetWidth(), s_imageViewer.GetHeight());
            card.Value(dimBuf, {.label = "Image Dimensions", .unit = "16-bit HDR", .col = Col::Full(), .size = UiSize::Small});

            card.Value(static_cast<double>(s_imageViewer.GetZoom() * 100.0f), {.label = "Zoom Level", .unit = "%", .format = "%.0f", .col = Col::Half(), .size = UiSize::Small});

            card.Value(static_cast<double>(s_imageViewer.GetRotation()), {.label = "Rotation", .unit = "\xC2\xB0", .format = "%.0f", .col = Col::Half(), .size = UiSize::Small});
        }
    }
}

} // namespace OmniKitShowcase
