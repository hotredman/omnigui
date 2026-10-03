#include "ui/components/UiTheme.hpp"
#include "core/Assets.hpp"
#include <imgui.h>
#include <algorithm>
#include <string>

UiTheme& UiTheme::Get() {
    static UiTheme instance;
    return instance;
}

UiTheme::UiTheme() {
    scale = 1.0f;   // масштаб задаёт приложение (SetScale), библиотека ОС не опрашивает
    ApplyPalette();
}

void UiTheme::SetMode(ThemeMode m) {
    mode = m;
    palette = ThemePalette::For(m);
    ApplyPalette();
    if (ImGui::GetCurrentContext()) {
        ApplyToImGui();
    }
}

// ============================================================================
// Стили компонентов из семантики палитры
// ============================================================================
void UiTheme::ApplyPalette() {
    const ThemePalette& p = palette;
    constexpr ImU32 kNone = 0;  // Прозрачный: элемент без подложки / рамки

    tab.colActiveBg     = p.accentBg;
    tab.colActiveText   = p.accent;
    tab.colInactiveText = p.textSecondary;
    tab.colHoverBg      = p.fillHover;
    tab.colHoverText    = p.textPrimary;
    tab.colSeparator    = p.divider;

    card.colBg     = p.bgSurface;
    card.colBorder = p.border;
    card.colTitle  = p.accent;

    input.colLabel  = p.textSecondary;
    input.colBg     = p.bgInset;
    input.colBorder = p.border;
    input.colText   = p.textPrimary;
    input.colUnit   = p.textMuted;

    combo.colLabel         = p.textSecondary;
    combo.colBg            = p.bgInset;
    combo.colBgHovered     = p.bgInset;
    combo.colBgActive      = p.bgInset;
    combo.colBorder        = p.border;
    combo.colBorderHovered = p.borderStrong;
    combo.colText          = p.textPrimary;
    combo.colArrow         = p.textMuted;
    combo.colArrowHovered  = p.textPrimary;
    combo.colPopupBg       = p.bgPopup;
    combo.colPopupBorder   = p.border;

    toggle.colTrackOff       = p.bgInset;
    toggle.colTrackOffHover  = p.bgInset;
    toggle.colTrackOffBorder = p.borderStrong;
    toggle.colTrackOn        = p.primary.solid;
    toggle.colTrackOnHover   = p.primary.solidHover;
    toggle.colTrackOnBorder  = p.primary.solid;
    toggle.colKnobOff        = p.textSecondary;
    toggle.colKnobOn         = p.textOnAccent;
    toggle.colTextMain       = p.textPrimary;
    toggle.colTextSub        = p.textSecondary;

    sidebar.colBg             = p.bgChrome;
    sidebar.colActiveBg       = p.accentBg;
    sidebar.colHoverBg        = p.fillHover;
    sidebar.colActiveText     = p.accent;
    sidebar.colInactiveText   = p.textSecondary;
    sidebar.colHoverText      = p.textPrimary;
    sidebar.colActiveBar      = p.accent;
    sidebar.colSeparator      = p.divider;
    sidebar.colScrollRegionBg = p.bgMuted;

    indicator.colTareBtn         = p.bgControl;
    indicator.colTareBtnHover    = p.bgControlHover;
    indicator.colTareBtnActive   = p.bgControlActive;
    indicator.colTareBorder      = p.border;
    indicator.colTareBorderHover = p.accent;
    indicator.colTareIcon        = p.textSecondary;
    indicator.colTareIconHover   = p.textPrimary;
    indicator.colBg              = p.bgSurface;
    indicator.colBorder          = p.border;
    indicator.colTitle           = p.textSecondary;
    indicator.colValue           = p.accent;
    indicator.colUnit            = p.textSecondary;

    valueDisplay.colBg     = p.bgInset;
    valueDisplay.colBorder = p.border;
    valueDisplay.colValue  = p.textPrimary;
    valueDisplay.colUnit   = p.textSecondary;

    header.colBg        = p.bgChrome;
    header.colSeparator = p.divider;

    statusBar.colBg        = p.bgChrome;
    statusBar.colTopLine   = p.divider;
    statusBar.colSeparator = p.divider;
    statusBar.colText      = p.textSecondary;

    deviceStatus.colBg              = p.bgSurface;
    deviceStatus.colBorder          = p.border;
    deviceStatus.colTitle           = p.textSecondary;
    deviceStatus.colStatusText      = p.textPrimary;
    deviceStatus.colLedDisconnected = p.signalOff;
    deviceStatus.colLedConnecting   = p.warning.solid;
    deviceStatus.colLedIdle         = p.success.solid;
    deviceStatus.colLedRunning      = p.primary.solid;
    deviceStatus.colLedPaused       = p.warning.solid;
    deviceStatus.colLedFault        = p.danger.solid;

    carousel.colBg        = p.bgSurface;
    carousel.colArrowBg   = p.bgControl;
    carousel.colArrowText = p.textSecondary;

    contextMenu.colBg         = p.bgPopup;
    contextMenu.colBorder     = p.border;
    contextMenu.colHeader     = p.textMuted;
    contextMenu.colText       = p.textPrimary;
    contextMenu.colTextActive = p.accent;
    contextMenu.colHoverBg    = p.fillHover;
    contextMenu.colActiveBg   = p.accentBg;
    contextMenu.colCheckmark  = p.accent;
    contextMenu.colSeparator  = p.divider;

    presetGrid.colDisplayBg       = p.bgInset;
    presetGrid.colDisplayBorder   = p.border;
    presetGrid.colDisplayValue    = p.textPrimary;
    presetGrid.colDisplayUnit     = p.textSecondary;
    presetGrid.colBtnBg           = p.bgControl;
    presetGrid.colBtnBorder       = p.borderSubtle;
    presetGrid.colBtnText         = p.textPrimary;
    presetGrid.colBtnHoverBg      = p.bgControlHover;
    presetGrid.colBtnHoverBorder  = p.borderStrong;
    presetGrid.colBtnHoverText    = p.textPrimary;
    presetGrid.colBtnActiveBg     = p.primary.solid;
    presetGrid.colBtnActiveBorder = p.primary.solid;
    presetGrid.colBtnActiveText   = p.textOnAccent;

    sidePanel.colBg             = kNone;
    sidePanel.colBorder         = kNone;
    sidePanel.colSplitter       = p.divider;
    sidePanel.colSplitterHover  = p.accent;
    sidePanel.colSplitterActive = p.primary.solid;

    toolbar.colBg        = p.bgChrome;
    toolbar.colSeparator = p.divider;

    editableLabel.colIcon        = p.textMuted;
    editableLabel.colIconHover   = p.textPrimary;
    editableLabel.colIconActive  = p.accent;
    editableLabel.colBtnBg       = p.bgControl;
    editableLabel.colBtnBorder   = p.borderSubtle;
    editableLabel.colInputBg     = p.bgInset;
    editableLabel.colInputBorder = p.accent;

    toolButton.colBg             = p.bgControl;
    toolButton.colBgHover        = p.bgControlHover;
    toolButton.colBgActive       = p.bgControlActive;
    toolButton.colBorder         = p.borderSubtle;
    toolButton.colBorderHover    = p.borderStrong;
    toolButton.colIcon           = p.textSecondary;
    toolButton.colIconHover      = p.textPrimary;
    toolButton.colBgSelected     = p.accentBg;
    toolButton.colBorderSelected = p.accent;

    table.colHeaderBg      = p.bgMuted;
    table.colHeaderHovered = p.fillHover;
    table.colHeaderActive  = p.accentBg;
    table.colHeaderText    = p.textSecondary;
    table.colBodyBg        = p.bgSurface;
    table.colRowBg         = kNone;
    table.colRowBgAlt      = p.fillSubtle;
    table.colRowHovered    = p.fillHover;
    table.colRowSelected   = p.accentBg;
    table.colBorderOuter   = p.border;
    table.colBorderInner   = p.divider;
    table.colEmptyIcon     = p.textMuted;
    table.colEmptyText     = p.textSecondary;

    filterBar.colLabel = p.textSecondary;

    scrollbar.colBg          = kNone;
    scrollbar.colBgHovered   = p.fillSubtle;
    scrollbar.colGrab        = p.scrollThumb;
    scrollbar.colGrabHovered = p.scrollThumbHover;
    scrollbar.colGrabActive  = p.scrollThumbActive;

    list.colBg             = kNone;
    list.colItemBg         = kNone;
    list.colItemHoverBg    = p.fillHover;
    list.colItemSelectedBg = p.accentBg;
    list.colActiveBar      = p.accent;
    list.colHeader         = p.textSecondary;
    list.colText           = p.textPrimary;
    list.colTextSelected   = p.textPrimary;
    list.colTextMuted      = p.textMuted;
    list.colBorder         = p.border;

    tag.colBg          = p.bgControl;
    tag.colBgHover     = p.bgControlHover;
    tag.colBgActive    = p.bgControlActive;
    tag.colBorder      = p.borderSubtle;
    tag.colBorderHover = p.accentBorder;
    tag.colPrefix      = p.textMuted;
    tag.colPrefixHover = p.accent;
    tag.colText        = p.textSecondary;
    tag.colTextHover   = p.textPrimary;

    itemRow.colBg              = p.fillSubtle;
    itemRow.colBgHover         = p.fillHover;
    itemRow.colBorder          = p.borderSubtle;
    itemRow.colBorderHover     = p.borderStrong;
    itemRow.colTextDescription = p.textSecondary;
    itemRow.colTextFormula     = p.accent;

    chart.colBg            = p.chartBg;
    chart.colBorder        = p.border;
    chart.colLine          = p.chartLine;
    chart.colMajorGrid     = p.chartGridMajor;
    chart.colMinorGrid     = p.chartGridMinor;
    chart.colAxis          = p.chartAxis;
    chart.colTickText      = p.chartText;
    chart.colAxisTitle     = p.chartText;
    chart.colCrosshair     = p.chartCrosshair;
    chart.colProjection    = p.chartProjection;
    chart.colTooltipBg     = p.tooltipBg;
    chart.colTooltipBorder = p.accentBorder;
    chart.colTooltipText   = p.tooltipText;
    chart.tickFontSize      = 18.0f;
    chart.axisTitleFontSize = 20.0f;

    // Бейдж: тинт-подложка, рамка и текст варианта
    auto badgeOf = [](const ThemePalette::Status& s) { return BadgeVariantColors{s.bg, s.border, s.text}; };
    badge.colors[static_cast<int>(UiVariant::Default)]   = {p.bgControl, p.borderSubtle, p.textPrimary};
    badge.colors[static_cast<int>(UiVariant::Primary)]   = badgeOf(p.primary);
    badge.colors[static_cast<int>(UiVariant::Secondary)] = {p.fillSubtle, p.borderSubtle, p.textSecondary};
    badge.colors[static_cast<int>(UiVariant::Success)]   = badgeOf(p.success);
    badge.colors[static_cast<int>(UiVariant::Warning)]   = badgeOf(p.warning);
    badge.colors[static_cast<int>(UiVariant::Danger)]    = badgeOf(p.danger);
    badge.colors[static_cast<int>(UiVariant::Info)]      = badgeOf(p.info);

    // Семантические варианты: текст варианта на поверхности, текст на
    // заливке, рамка валидации, заливка кнопки (покой / наведение / нажатие)
    auto neutral = [&p](ImU32 text) {
        return SemanticStyle{text, p.textPrimary, p.border, ImColor(p.bgControl).Value,
                             ImColor(p.bgControlHover).Value, ImColor(p.bgControlActive).Value};
    };
    auto filled = [&p](const ThemePalette::Status& s) {
        return SemanticStyle{s.text, p.textOnAccent, s.solid, ImColor(s.solid).Value,
                             ImColor(s.solidHover).Value, ImColor(s.solidActive).Value};
    };
    m_variants[static_cast<int>(UiVariant::Default)]   = neutral(p.textPrimary);
    m_variants[static_cast<int>(UiVariant::Primary)]   = filled(p.primary);
    m_variants[static_cast<int>(UiVariant::Secondary)] = neutral(p.textSecondary);
    m_variants[static_cast<int>(UiVariant::Success)]   = filled(p.success);
    m_variants[static_cast<int>(UiVariant::Warning)]   = filled(p.warning);
    m_variants[static_cast<int>(UiVariant::Danger)]    = filled(p.danger);
    m_variants[static_cast<int>(UiVariant::Info)]      = filled(p.info);
}

void UiTheme::LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();

    auto loadTypeface = [&](const std::string& assetRelPath) -> ImFont* {
        std::string fullPath = Assets::Resolve(assetRelPath);
        ImFont* font = nullptr;
        // В Dear ImGui 1.92+ номинальный размер 16px задает font->LegacySize.
        // Глифы динамически растрируются FreeType по требованию под любой размер из PushFont(font, size).
        if (!fullPath.empty()) {
            font = io.Fonts->AddFontFromFileTTF(fullPath.c_str(), 16.0f);
        }
        return font;
    };

    fontRegular = loadTypeface("fonts/Roboto-Regular.ttf");
    fontMedium  = loadTypeface("fonts/Roboto-Medium.ttf");
    fontBold    = loadTypeface("fonts/Roboto-Bold.ttf");

    if (!fontRegular) fontRegular = io.Fonts->AddFontDefault();
    if (!fontMedium)  fontMedium  = fontRegular;
    if (!fontBold)    fontBold    = fontMedium;

    // Алиасы для обратной совместимости
    defaultFont      = fontRegular;
    smallFont        = fontRegular;
    buttonFont       = fontMedium;
    projectTitleFont = fontBold;
    displayBigFont   = fontBold;

    // Привязываем шрифты к дескрипторам компонентов
    tab.font            = fontMedium;
    sidebar.font        = fontMedium;
    sidebar.activeFont  = fontMedium;
    card.titleFont      = fontBold;
    input.labelFont     = fontRegular;
    input.valueFont     = fontMedium;
    input.unitFont      = fontRegular;
    combo.labelFont     = fontRegular;
    combo.font          = fontMedium;
    toggle.fontLabel    = fontMedium;
    toggle.fontSublabel = fontRegular;
    indicator.titleFont = fontMedium;
    indicator.valueFont = fontBold;
    indicator.unitFont  = fontMedium;
    valueDisplay.valueFont = fontBold;
    valueDisplay.unitFont  = fontRegular;
    deviceStatus.titleFont  = fontMedium;
    deviceStatus.statusFont = fontBold;
    contextMenu.headerFont  = fontBold;
    contextMenu.itemFont    = fontMedium;
    presetGrid.displayValFont  = fontBold;
    presetGrid.displayUnitFont = fontRegular;
    presetGrid.btnFont         = fontMedium;
    toolButton.font     = fontMedium;
    list.headerFont     = fontBold;
    list.itemFont       = fontMedium;
    list.subFont        = fontRegular;
    itemRow.titleFont       = fontBold;
    itemRow.symbolFont      = fontBold;
    itemRow.unitFont        = fontMedium;
    itemRow.descriptionFont = fontRegular;
    itemRow.formulaFont     = fontMedium;
    tag.font                = fontMedium;

    // Устанавливаем базовый шрифт по умолчанию для всего ImGui
    io.FontDefault = fontRegular;
}

void UiTheme::SetScale(float s) {
    scale = std::clamp(s, kMinScale, kMaxScale);
    if (ImGui::GetCurrentContext()) {
        ApplyToImGui();
    }
}

void UiTheme::ApplyToImGui() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.FontSizeBase  = Scale(18.0f);
    style.FontScaleMain = 1.0f;
    style.FontScaleDpi  = 1.0f;

    // Скругления
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = Scale(card.cornerRadius);
    style.FrameRounding     = Scale(input.frameRounding);
    style.PopupRounding     = CornerRadius();
    style.ScrollbarRounding = Scale(scrollbar.cornerRadius);
    style.GrabRounding      = Scale(scrollbar.cornerRadius);
    style.TabRounding       = CornerRadius();

    // Размеры скроллбара
    style.ScrollbarSize     = Scale(scrollbar.size);
    style.GrabMinSize       = Scale(scrollbar.minGrabSize);

    // Толщина рамок
    style.WindowBorderSize  = 0.0f;
    style.ChildBorderSize   = 0.0f;
    style.FrameBorderSize   = input.borderSize;
    style.PopupBorderSize   = 1.0f;
    style.TabBorderSize     = 0.0f;

    // Внутренние и внешние отступы
    style.WindowPadding     = ImVec2(SpacingMedium(), SpacingMedium());
    style.ItemSpacing       = ImVec2(SpacingMedium(), SpacingMedium());
    style.ItemInnerSpacing  = ImVec2(SpacingSmall(), SpacingSmall());
    style.CellPadding       = ImVec2(SpacingSmall(), SpacingSmall());
    style.FramePadding      = ImVec2(Scale(input.framePaddingX), Scale(input.framePaddingY));
    style.DisabledAlpha     = kDisabledAlpha;

    const ThemePalette& p = palette;
    auto set = [&style](ImGuiCol idx, ImU32 color) { style.Colors[idx] = ImColor(color).Value; };

    set(ImGuiCol_WindowBg, p.bgApp);
    // Дочернее окно — область, а не поверхность: фон рисует только тот
    // компонент, которому он нужен (Card, List, SidePanel...), явно
    set(ImGuiCol_ChildBg, 0);
    set(ImGuiCol_PopupBg, p.bgPopup);
    set(ImGuiCol_Border, p.border);
    set(ImGuiCol_BorderShadow, 0);

    // Модальные окна и затемнение под ними
    set(ImGuiCol_ModalWindowDimBg, p.overlay);
    set(ImGuiCol_TitleBg, p.bgChrome);
    set(ImGuiCol_TitleBgActive, p.bgChrome);
    set(ImGuiCol_TitleBgCollapsed, p.bgChrome);

    // Скроллбар
    set(ImGuiCol_ScrollbarBg, scrollbar.colBg);
    set(ImGuiCol_ScrollbarGrab, scrollbar.colGrab);
    set(ImGuiCol_ScrollbarGrabHovered, scrollbar.colGrabHovered);
    set(ImGuiCol_ScrollbarGrabActive, scrollbar.colGrabActive);

    // Поля ввода (Frames)
    set(ImGuiCol_FrameBg, p.bgInset);
    set(ImGuiCol_FrameBgHovered, p.bgInset);
    set(ImGuiCol_FrameBgActive, p.bgInset);
    set(ImGuiCol_TextSelectedBg, p.accentBorder);

    // Чекбоксы, слайдеры
    set(ImGuiCol_CheckMark, p.accent);
    set(ImGuiCol_SliderGrab, p.primary.solid);
    set(ImGuiCol_SliderGrabActive, p.primary.solidHover);

    // Кнопки ImGui по умолчанию — нейтральный контрол
    set(ImGuiCol_Button, p.bgControl);
    set(ImGuiCol_ButtonHovered, p.bgControlHover);
    set(ImGuiCol_ButtonActive, p.bgControlActive);

    // Списки, меню, выделение строк
    set(ImGuiCol_Header, p.accentBg);
    set(ImGuiCol_HeaderHovered, p.fillHover);
    set(ImGuiCol_HeaderActive, p.accentBg);

    // Текст
    set(ImGuiCol_Text, p.textPrimary);
    set(ImGuiCol_TextDisabled, p.textMuted);

    // Разделители
    set(ImGuiCol_Separator, p.divider);
    set(ImGuiCol_SeparatorHovered, p.borderStrong);
    set(ImGuiCol_SeparatorActive, p.accent);

    // Таблицы
    set(ImGuiCol_TableHeaderBg, table.colHeaderBg);
    set(ImGuiCol_TableRowBg, table.colRowBg);
    set(ImGuiCol_TableRowBgAlt, table.colRowBgAlt);
    set(ImGuiCol_TableBorderStrong, table.colBorderOuter);
    set(ImGuiCol_TableBorderLight, table.colBorderInner);

    // Вкладки (Tabs)
    set(ImGuiCol_Tab, p.bgControl);
    set(ImGuiCol_TabHovered, p.fillHover);
    set(ImGuiCol_TabSelected, p.accentBg);
    set(ImGuiCol_TabSelectedOverline, p.accent);
    set(ImGuiCol_TabDimmed, p.bgControl);
    set(ImGuiCol_TabDimmedSelected, p.accentBg);
    set(ImGuiCol_TabDimmedSelectedOverline, p.accent);

    // Навигация и ресайз (контур фокуса скрыт — без паразитного двойного контура)
    set(ImGuiCol_NavCursor, 0);
    set(ImGuiCol_ResizeGrip, 0);
    set(ImGuiCol_ResizeGripHovered, p.borderStrong);
    set(ImGuiCol_ResizeGripActive, p.accent);
}

ControlMetrics UiTheme::GetMetrics(UiSize size) const {
    ControlMetrics m;
    switch (size) {
        case UiSize::Large:
            m.height   = Scale(48.0f);
            m.paddingX = Scale(16.0f);
            m.paddingY = Scale(10.0f);
            m.rounding = Scale(8.0f);
            m.iconSize = Scale(22.0f);
            m.font     = fontBold ? fontBold : defaultFont;
            m.fontSize = 22.0f;
            break;
        case UiSize::Medium:
            m.height   = Scale(36.0f);
            m.paddingX = Scale(12.0f);
            m.paddingY = Scale(6.0f);
            m.rounding = Scale(6.0f);
            m.iconSize = Scale(16.0f);
            m.font     = fontMedium ? fontMedium : defaultFont;
            m.fontSize = 18.0f;
            break;
        case UiSize::Small:
            m.height   = Scale(30.0f);
            m.paddingX = Scale(10.0f);
            m.paddingY = Scale(4.0f);
            m.rounding = Scale(5.0f);
            m.iconSize = Scale(14.0f);
            m.font     = fontMedium ? fontMedium : defaultFont;
            m.fontSize = 16.0f;
            break;
        case UiSize::Mini:
            m.height   = Scale(26.0f);
            m.paddingX = Scale(7.0f);
            m.paddingY = Scale(2.0f);
            m.rounding = Scale(4.0f);
            m.iconSize = Scale(12.0f);
            m.font     = fontMedium ? fontMedium : defaultFont;
            m.fontSize = 14.0f;
            break;
    }
    return m;
}

float UiTheme::SizeFactor(UiSize size) const {
    return GetMetrics(size).fontSize / GetMetrics(UiSize::Medium).fontSize;
}

const SemanticStyle& UiTheme::GetVariantStyle(UiVariant variant) const {
    const size_t idx = static_cast<size_t>(variant);
    return idx < std::size(m_variants) ? m_variants[idx] : m_variants[0];
}
