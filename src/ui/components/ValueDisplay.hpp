#pragma once

#include "ui/components/UiTheme.hpp"

// Параметры табло значения (designated initializers):
//
//     ValueDisplay(42.5, {.unit = "Hz"});
//     ValueDisplay(1280.0, {.unit = "points", .format = "%.0f", .size = UiSize::Small});
struct ValueDisplayOptions {
    const char* unit    = nullptr;    // единица измерения
    const char* format  = "%.2f";     // printf-формат (только для числового значения)
    UiSize      size    = UiSize::Medium; // высота и кегль по шкале UiSize (высота — как у полей ввода)
    const ValueDisplayStyle* style = nullptr;  // оверрайд стиля; nullptr — из темы

    // Внутренний механизм для контейнеров (Card, PresetGrid), которые сами считают
    // геометрию: финальные px, 0 — авто (вся ширина / высота по умолчанию). Не для кода приложения.
    ImVec2      sizePx  = ImVec2(0.0f, 0.0f);
};

// Числовое значение с единицей измерения и форматированием
void ValueDisplay(double value, const ValueDisplayOptions& options = {});

// Строковое значение с единицей измерения (options.format игнорируется)
void ValueDisplay(const char* text, const ValueDisplayOptions& options = {});
