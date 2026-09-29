#pragma once

#include <imgui.h>
enum class ThemeMode { Dark, Light };

// ============================================================================
// Палитра темы оформления (канон evo-machine-cs: tokens.css + theme-*.css).
//
// Слой 1 — примитивы: тема-независимые оттенки (шкалы slate/sky/emerald/
// amber/red). Слой 2 — ThemePalette: семантика (фон приложения, поверхность,
// рамка, текст, акцент, статусы, графики), у тёмной и светлой темы свои
// значения. Слой 3 — стили компонентов в UiTheme собираются только из
// семантики палитры: смена темы = смена палитры, компоненты цветов не хранят.
// ============================================================================

// Слой 1: примитивы
namespace Tone {
constexpr ImU32 Rgba(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }
// Тот же оттенок с прозрачностью (0..1)
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
constexpr ImU32 Slate850 = Rgba(22, 32, 50);   // Промежуточный (evo-machine-cs: slate-hover-dark)
constexpr ImU32 Slate900 = Rgba(15, 23, 42);
constexpr ImU32 Slate925 = Rgba(11, 19, 41);   // Каркас тёмной темы (evo-machine-cs: bg-row-deep)
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

// Слой 2: семантика темы
struct ThemePalette {
    // Статус: заливка (кнопка, светодиод) с hover/нажатием, текст на
    // поверхности, тинт-подложка и рамка (бейдж, баннер)
    struct Status {
        ImU32 solid;
        ImU32 solidHover;
        ImU32 solidActive;
        ImU32 text;
        ImU32 bg;
        ImU32 border;
    };

    // Поверхности
    ImU32 bgApp;            // Холст рабочей области
    ImU32 bgChrome;         // Каркас: шапка, полоса проекта, сайдбар, строка состояния
    ImU32 bgSurface;        // Карточка, панель
    ImU32 bgMuted;          // Приглушённая подложка на поверхности (шапка таблицы)
    ImU32 bgInset;          // Поле ввода, табло, холст графика
    ImU32 bgPopup;          // Выпадающий список, меню, модальное окно
    ImU32 bgControl;        // Нейтральный контрол / чип в покое (полупрозрачный)
    ImU32 bgControlHover;
    ImU32 bgControlActive;
    ImU32 fillSubtle;       // Подложка элемента на поверхности (строка-карточка, зебра)
    ImU32 fillHover;        // Подсветка строки / пункта при наведении
    ImU32 overlay;          // Затемнение под модальным окном

    // Границы
    ImU32 border;           // Рамка поверхности и контрола
    ImU32 borderStrong;     // Рамка при наведении
    ImU32 borderSubtle;     // Рамка чипа, строки-карточки
    ImU32 divider;          // Разделители

    // Текст
    ImU32 textPrimary;
    ImU32 textSecondary;    // Подписи, вторичный текст
    ImU32 textMuted;        // Подсказки, заглушки
    ImU32 textDisabled;
    ImU32 textOnAccent;     // Текст на цветной заливке

    // Акцент (бренд, выбор, фокус) и статусы
    ImU32 accent;           // Текст / иконка / полоска активного
    ImU32 accentBg;         // Тинт выбранного
    ImU32 accentBorder;
    Status primary;         // Основное действие (заливка кнопки, тумблер)
    Status success;
    Status warning;
    Status danger;
    Status info;
    ImU32 signalOff;        // Светодиод: нет связи / выключен

    // Графики
    ImU32 chartBg;
    ImU32 chartGridMajor;
    ImU32 chartGridMinor;
    ImU32 chartAxis;
    ImU32 chartText;
    ImU32 chartLine;        // Кривая испытания
    ImU32 chartCrosshair;
    ImU32 chartProjection;  // Проекции маркеров на оси
    ImU32 tooltipBg;
    ImU32 tooltipText;

    // Полоса прокрутки
    ImU32 scrollThumb;
    ImU32 scrollThumbHover;
    ImU32 scrollThumbActive;

    static ThemePalette Dark();
    static ThemePalette Light();
    static ThemePalette For(ThemeMode mode) { return mode == ThemeMode::Light ? Light() : Dark(); }
};
