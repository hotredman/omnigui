#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/UiKey.hpp"
#include <imgui.h>
#include <source_location>

// Параметры кнопки. Агрегат: поля задаются по именам (designated initializers),
// порядок — как в объявлении, пропущенные поля берут значения по умолчанию.
// Кнопка может быть без подписи (только иконка):
//
//     if (Button({.label = "Save", .variant = UiVariant::Primary, .icon = Icon::Plus})) { ... }
//     Button({.icon = Icon::Cog, .tooltip = "Settings"});
//     Button("OK");   // короткая форма: только подпись
//
// Идентичность кнопки (ImGui ID) автоматически выводится из места вызова (source_location)
// и подписи/иконки; при необходимости задаётся явный ключ key (строка, int или указатель).
struct ButtonOptions {
    const char* label    = nullptr;           // подпись; nullptr — кнопка без текста
    UiVariant   variant  = UiVariant::Default;
    Icon        icon     = Icon::None;
    UiSize      size     = UiSize::Medium;    // высота кнопки по дизайн-системе
    float       width    = 0.0f;              // базовые px (до масштаба); 0 — по содержимому; Fill — на всю ширину
    const char* tooltip  = nullptr;           // подсказка при наведении
    bool        disabled = false;
    UiKey       key      = {};                // явная идентичность: строка, int или ptr
};

// Кнопка дизайн-системы. Возвращает true в кадре клика.
bool Button(const ButtonOptions& options = {},
            std::source_location loc = std::source_location::current());
bool Button(const char* label,
            std::source_location loc = std::source_location::current());

// Естественная ширина кнопки по содержимому (подпись, иконка, отступы темы), px
float ButtonWidth(const ButtonOptions& options);

// Для компонентов-контейнеров (Card, панели), которые уже посчитали геометрию:
// размер в итоговых пикселях (с учётом масштаба); ось, заданная нулём,
// берётся из options (ширина — width, высота — size).
// Приложению нужны Button/ButtonWidth.
bool ButtonPx(ImVec2 sizePx, const ButtonOptions& options = {},
              std::source_location loc = std::source_location::current());

// Ширина по содержимому для кнопки заданной высоты в итоговых px (0 — высота Medium)
float ButtonWidthPx(const ButtonOptions& options, float heightPx);
