#include "ui/components/ThemePalette.hpp"

using namespace Tone;

// Тёмная тема: каркас темнее холста, карточки светлее холста; акценты и
// статусы — яркие 400/500-е оттенки
ThemePalette ThemePalette::Dark() {
    ThemePalette p{};
    p.bgApp           = Slate900;
    p.bgChrome        = Slate925;
    p.bgSurface       = Slate800;
    p.bgMuted         = Slate850;
    p.bgInset         = Slate900;
    p.bgPopup         = Slate800;
    p.bgControl       = Alpha(Slate500, 0.18f);
    p.bgControlHover  = Alpha(Slate500, 0.30f);
    p.bgControlActive = Alpha(Slate500, 0.40f);
    p.fillSubtle      = Alpha(White, 0.03f);
    p.fillHover       = Alpha(White, 0.06f);
    p.overlay         = Alpha(Slate950, 0.60f);

    p.border       = Slate700;
    p.borderStrong = Slate600;
    p.borderSubtle = Alpha(Slate500, 0.30f);
    p.divider      = Slate700;

    p.textPrimary   = Slate100;
    p.textSecondary = Slate400;
    p.textMuted     = Slate500;
    p.textDisabled  = Slate600;
    p.textOnAccent  = White;

    p.accent       = Sky400;
    p.accentBg     = Alpha(Sky400, 0.14f);
    p.accentBorder = Alpha(Sky400, 0.35f);
    p.primary = {Sky600, Sky500, Sky700, Sky400, Alpha(Sky400, 0.14f), Alpha(Sky400, 0.35f)};
    p.success = {Emerald600, Emerald500, Emerald700, Emerald400, Alpha(Emerald500, 0.14f), Alpha(Emerald500, 0.35f)};
    p.warning = {Amber600, Amber500, Amber700, Amber400, Alpha(Amber500, 0.14f), Alpha(Amber500, 0.35f)};
    p.danger  = {Red600, Red500, Red700, Red400, Alpha(Red500, 0.15f), Alpha(Red500, 0.35f)};
    p.info    = {Sky600, Sky500, Sky700, Sky400, Alpha(Sky400, 0.14f), Alpha(Sky400, 0.35f)};
    p.signalOff = Slate500;

    p.chartBg         = Slate900;
    p.chartGridMajor  = Slate800;
    p.chartGridMinor  = Alpha(Slate800, 0.50f);
    p.chartAxis       = Slate600;
    p.chartText       = Slate400;
    p.chartLine       = Sky400;
    p.chartCrosshair  = Alpha(Slate400, 0.50f);
    p.chartProjection = Alpha(Slate400, 0.45f);
    p.tooltipBg       = Slate900;
    p.tooltipText     = Slate100;

    p.scrollThumb       = Alpha(Slate400, 0.35f);
    p.scrollThumbHover  = Alpha(Slate400, 0.60f);
    p.scrollThumbActive = Sky400;
    return p;
}

// Светлая тема: не инверсия, а дизайн-проход — белые карточки на светло-
// сером холсте, акценты и статусы на тон темнее (600/700), тексты тёмных
// оттенков для контраста
ThemePalette ThemePalette::Light() {
    ThemePalette p{};
    p.bgApp           = Slate100;
    p.bgChrome        = White;
    p.bgSurface       = White;
    p.bgMuted         = Slate50;
    p.bgInset         = White;
    p.bgPopup         = White;
    p.bgControl       = Alpha(Slate500, 0.12f);
    p.bgControlHover  = Alpha(Slate500, 0.22f);
    p.bgControlActive = Alpha(Slate500, 0.32f);
    p.fillSubtle      = Alpha(Slate900, 0.025f);
    p.fillHover       = Alpha(Slate900, 0.05f);
    p.overlay         = Alpha(Slate900, 0.35f);

    p.border       = Slate300;
    p.borderStrong = Slate400;
    p.borderSubtle = Alpha(Slate500, 0.30f);
    p.divider      = Slate200;

    p.textPrimary   = Slate900;
    p.textSecondary = Slate600;
    p.textMuted     = Slate500;
    p.textDisabled  = Slate400;
    p.textOnAccent  = White;

    p.accent       = Sky700;
    p.accentBg     = Alpha(Sky600, 0.10f);
    p.accentBorder = Alpha(Sky600, 0.40f);
    p.primary = {Sky600, Sky700, Sky800, Sky700, Alpha(Sky600, 0.10f), Alpha(Sky600, 0.40f)};
    p.success = {Emerald600, Emerald700, Emerald800, Emerald700, Alpha(Emerald500, 0.10f), Alpha(Emerald600, 0.40f)};
    p.warning = {Amber600, Amber700, Amber800, Amber700, Alpha(Amber500, 0.12f), Alpha(Amber600, 0.40f)};
    p.danger  = {Red600, Red700, Red800, Red700, Alpha(Red500, 0.08f), Alpha(Red600, 0.40f)};
    p.info    = {Sky600, Sky700, Sky800, Sky700, Alpha(Sky600, 0.10f), Alpha(Sky600, 0.40f)};
    p.signalOff = Slate400;

    p.chartBg         = White;
    p.chartGridMajor  = Slate200;
    p.chartGridMinor  = Slate100;
    p.chartAxis       = Slate400;
    p.chartText       = Slate600;
    p.chartLine       = Sky600;
    p.chartCrosshair  = Alpha(Slate600, 0.50f);
    p.chartProjection = Alpha(Slate600, 0.40f);
    p.tooltipBg       = White;
    p.tooltipText     = Slate900;

    p.scrollThumb       = Alpha(Slate500, 0.40f);
    p.scrollThumbHover  = Alpha(Slate500, 0.65f);
    p.scrollThumbActive = Sky600;
    return p;
}
