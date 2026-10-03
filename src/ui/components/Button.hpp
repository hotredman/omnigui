#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>

// Параметры кнопки. Агрегат: поля задаются по именам (designated initializers),
// порядок — как в объявлении, пропущенные поля берут значения по умолчанию:
//
//     if (Button("Save", {.variant = UiVariant::Primary, .icon = Icon::Plus})) { ... }
//     Button("Delete", {.variant = UiVariant::Danger, .disabled = !canDelete});
struct ButtonOptions {
    // Ширина на всю доступную ширину контейнера
    static constexpr float Fill = -1.0f;

    UiVariant variant  = UiVariant::Default;
    Icon      icon     = Icon::None;
    UiSize    size     = UiSize::Medium;  // высота кнопки по дизайн-системе
    float     width    = 0.0f;            // базовые px (до масштаба); 0 — по содержимому; Fill — на всю ширину
    bool      disabled = false;
};

// Кнопка дизайн-системы. Возвращает true в кадре клика.
// Метка может содержать "##id" — всё после него в тексте не показывается.
bool Button(const char* label, const ButtonOptions& options = {});

// Естественная ширина кнопки по содержимому (текст, иконка, отступы темы), px
float ButtonWidth(const char* label, const ButtonOptions& options = {});

// Кнопка-иконка без текста (квадратная, сторона — высота options.size)
bool IconButton(const char* id, Icon icon, const ButtonOptions& options = {});

// Для компонентов-контейнеров (Card, панели), которые уже посчитали геометрию:
// размер в итоговых пикселях (с учётом масштаба); ось, заданная нулём,\n// берётся из options (ширина — width, высота — size).
// Приложению нужны Button/ButtonWidth.
bool ButtonPx(const char* label, ImVec2 sizePx, const ButtonOptions& options = {});

// Ширина по содержимому для кнопки заданной высоты в итоговых px (0 — высота Medium)
float ButtonWidthPx(const char* label, Icon icon, float heightPx);
