#pragma once

#include <imgui.h>

namespace OmniKit {
enum class ThemeMode { Dark, Light };

// ============================================================================
// Theme palette (canon evo-machine-cs: tokens.css + theme-*.css).
//
// Layer 1 — Primitives: theme-independent shades (slate/sky/emerald/
// amber/red scales). Layer 2 — ThemePalette: semantics (application background,
// surface, border, text, accent, statuses, charts), with dark and light themes
// having their own values. Layer 3 — Component styles in UiTheme are built
// exclusively from palette semantics: changing theme = changing palette;
// components do not store raw colors.
// ============================================================================

// Layer 1: Primitives
namespace Tone {
constexpr ImU32 Rgba(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }
// Same shade with alpha transparency (0..1)
constexpr ImU32 Alpha(ImU32 color, float alpha) {
    return (color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(alpha * 255.0f + 0.5f) << IM_COL32_A_SHIFT);
}

constexpr ImU32 White = Rgba(255, 255, 255);

constexpr ImU32 Slate50  = Rgba(248, 250, 252);
constexpr ImU32 Slate100 = Rgba(241, 245, 249);
constexpr ImU32 Slate200 = Rgba(226, 232, 240);
constexpr ImU32 Slate300 = Rgba(203, 213, 225);
constexpr ImU32 Slate400 = Rgba(148, 163, 184);
constexpr ImU32 Slate500 = Rgba(100, 116, 139);
constexpr ImU32 Slate600 = Rgba(71, 85, 105);
constexpr ImU32 Slate700 = Rgba(51, 65, 85);
constexpr ImU32 Slate800 = Rgba(30, 41, 59);
constexpr ImU32 Slate850 = Rgba(22, 32, 50);   // Intermediate (evo-machine-cs: slate-hover-dark)
constexpr ImU32 Slate900 = Rgba(15, 23, 42);
constexpr ImU32 Slate925 = Rgba(11, 19, 41);   // Dark theme frame (evo-machine-cs: bg-row-deep)
constexpr ImU32 Slate950 = Rgba(2, 6, 23);

constexpr ImU32 Sky300 = Rgba(186, 230, 253);
constexpr ImU32 Sky400 = Rgba(56, 189, 248);
constexpr ImU32 Sky500 = Rgba(14, 165, 233);
constexpr ImU32 Sky600 = Rgba(2, 132, 199);
constexpr ImU32 Sky700 = Rgba(3, 105, 161);
constexpr ImU32 Sky800 = Rgba(7, 89, 133);

constexpr ImU32 Emerald400 = Rgba(52, 211, 153);
constexpr ImU32 Emerald500 = Rgba(16, 185, 129);
constexpr ImU32 Emerald600 = Rgba(5, 150, 105);
constexpr ImU32 Emerald700 = Rgba(4, 120, 87);
constexpr ImU32 Emerald800 = Rgba(6, 95, 70);

constexpr ImU32 Amber400 = Rgba(251, 191, 36);
constexpr ImU32 Amber500 = Rgba(245, 158, 11);
constexpr ImU32 Amber600 = Rgba(217, 119, 6);
constexpr ImU32 Amber700 = Rgba(180, 83, 9);
constexpr ImU32 Amber800 = Rgba(146, 64, 14);

constexpr ImU32 Red400 = Rgba(248, 113, 113);
constexpr ImU32 Red500 = Rgba(239, 68, 68);
constexpr ImU32 Red600 = Rgba(220, 38, 38);
constexpr ImU32 Red700 = Rgba(185, 28, 28);
constexpr ImU32 Red800 = Rgba(153, 27, 27);
}  // namespace Tone

// Layer 2: Theme semantics
struct ThemePalette {
    // Status: solid fill (button, LED indicator) with hover/active states,
    // text on surface, tinted background and border (badge, banner)
    struct Status {
        ImU32 solid;
        ImU32 solidHover;
        ImU32 solidActive;
        ImU32 text;
        ImU32 bg;
        ImU32 border;
    };

    // Surfaces
    ImU32 bgApp;            // Workspace canvas background
    ImU32 bgChrome;         // Frame: header, project bar, sidebar, status bar
    ImU32 bgSurface;        // Card, panel
    ImU32 bgMuted;          // Muted surface background (e.g., table header)
    ImU32 bgInset;          // Input field, readout display, chart canvas
    ImU32 bgPopup;          // Dropdown, menu, modal window
    ImU32 bgControl;        // Neutral control / chip at rest (semi-transparent)
    ImU32 bgControlHover;
    ImU32 bgControlActive;
    ImU32 fillSubtle;       // Element background on surface (card row, zebra striping)
    ImU32 fillHover;        // Row / item highlight on hover
    ImU32 overlay;          // Dimming backdrop behind modal dialogs

    // Borders
    ImU32 border;           // Surface and control border
    ImU32 borderStrong;     // Border on hover / focus
    ImU32 borderSubtle;     // Chip, card-row border
    ImU32 divider;          // Dividers

    // Text
    ImU32 textPrimary;
    ImU32 textSecondary;    // Labels, secondary text
    ImU32 textMuted;        // Hints, placeholders
    ImU32 textDisabled;
    ImU32 textOnAccent;     // Text on colored solid fills

    // Accent (brand, selection, focus) and statuses
    ImU32 accent;           // Active text / icon / indicator bar
    ImU32 accentBg;         // Selected item tint background
    ImU32 accentBorder;
    Status primary;         // Primary action (button fill, toggle)
    Status success;
    Status warning;
    Status danger;
    Status info;
    ImU32 signalOff;        // LED: disconnected / inactive

    // Charts
    ImU32 chartBg;
    ImU32 chartGridMajor;
    ImU32 chartGridMinor;
    ImU32 chartAxis;
    ImU32 chartText;
    ImU32 chartLine;        // Test curve / plot series line
    ImU32 chartCrosshair;
    ImU32 chartProjection;  // Marker projections on axes
    ImU32 tooltipBg;
    ImU32 tooltipText;

    // Scrollbar
    ImU32 scrollThumb;
    ImU32 scrollThumbHover;
    ImU32 scrollThumbActive;

    static ThemePalette Dark();
    static ThemePalette Light();
    static ThemePalette For(ThemeMode mode) { return mode == ThemeMode::Light ? Light() : Dark(); }
};

} // namespace OmniKit
