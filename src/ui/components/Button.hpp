#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>

// Параметры кнопки. Агрегат: поля задаются по именам (designated initializers),
// порядок — как в объявлении, пропущенные поля берут значения по умолчанию.
// Кнопка может быть без подписи (только иконка):
//
//     if (Button({.label = "Save", .variant = UiVariant::Primary, .icon = Icon::Plus})) { ... }
//     Button({.icon = Icon::Cog, .tooltip = "Settings"});
//     Button("OK");   // короткая форма: только подпись
//
// Идентичность кнопки (ImGui ID) выводится из подписи и иконки; одинаковые
// кнопки в цикле различаются областью IdScope, при необходимости — полем key.
struct ButtonOptions {
    // Ширина на всю доступную ширину контейнера
    static constexpr float Fill = -1.0f;

    const char* label    = nullptr;           // подпись; nullptr — кнопка без текста
    UiVariant   variant  = UiVariant::Default;
    Icon        icon     = Icon::None;
    UiSize      size     = UiSize::Medium;    // высота кнопки по дизайн-системе
    float       width    = 0.0f;              // базовые px (до масштаба); 0 — по содержимому; Fill — на всю ширину
    const char* tooltip  = nullptr;           // подсказка при наведении
    bool        disabled = false;
    const char* key      = nullptr;           // явная идентичность, когда подписи и иконки недостаточно
};

// Кнопка дизайн-системы. Возвращает true в кадре клика.
bool Button(const ButtonOptions& options);
bool Button(const char* label);

// Естественная ширина кнопки по содержимому (подпись, иконка, отступы темы), px
float ButtonWidth(const ButtonOptions& options);

// Для компонентов-контейнеров (Card, панели), которые уже посчитали геометрию:
// размер в итоговых пикселях (с учётом масштаба); ось, заданная нулём,
// берётся из options (ширина — width, высота — size).
// Приложению нужны Button/ButtonWidth.
bool ButtonPx(ImVec2 sizePx, const ButtonOptions& options);

// Ширина по содержимому для кнопки заданной высоты в итоговых px (0 — высота Medium)
float ButtonWidthPx(const ButtonOptions& options, float heightPx);
