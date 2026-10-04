#pragma once

#include "ui/components/image/ImageStyle.hpp"
#include "ui/components/UiTheme.hpp"

namespace OmniKit {

// ============================================================================
// Адаптер дизайн-системы для подсистемы Image
// Чистое связывание (Adapter Pattern): ни UiTheme, ни ImageViewer не знают
// друг о друге, конвертация токенов происходит здесь.
// ============================================================================
inline ImageViewerStyle MakeImageViewerStyle(const UiTheme& theme) {
    const auto& p = theme.palette;
    ImageViewerStyle s;
    s.colBg               = p.bgApp;
    s.colBorder           = p.border;
    s.colImageBorder      = p.borderSubtle;
    s.colSliceLine        = p.danger.solid;
    s.colSliceHandle      = p.danger.solid;
    s.colOverlayBg        = Tone::Alpha(p.bgSurface, 0.85f);
    s.colOverlayBorder    = p.borderSubtle;
    s.colOverlayText      = p.textPrimary;
    s.colOverlaySecondary = p.textSecondary;
    s.colProfileLine      = p.accent;
    s.colDivider          = p.divider;
    s.cornerRadius        = theme.CornerRadius();
    s.scale               = theme.GetScale();
    return s;
}

inline ImageHistogramStyle MakeImageHistogramStyle(const UiTheme& theme) {
    const auto& p = theme.palette;
    ImageHistogramStyle s;
    s.colBg        = p.bgInset;
    s.colBorder    = p.border;
    s.colBins      = Tone::Alpha(p.textSecondary, 0.70f);
    s.colBand      = p.accentBg;
    s.colLines     = p.accent;
    s.colHandles   = p.accent;
    s.cornerRadius = theme.CornerRadius();
    s.scale        = theme.GetScale();
    return s;
}

} // namespace OmniKit
