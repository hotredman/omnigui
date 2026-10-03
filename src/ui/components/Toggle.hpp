#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <string>

// =========================================================================================
// Компонент переключателя Toggle (современный тумблер-слайдер в стиле @evo/ui/Toggle.svelte)
// =========================================================================================

// Параметры переключателя (designated initializers):
//
//     Toggle(enabled, {.label = "Auto", .sublabel = "Adjusts on its own"});
//
// Идентичность выводится из подписи; без подписи (или для одинаковых подписей)
// задаётся key либо область IdScope.
struct ToggleOptions {
    const char* label    = nullptr;
    const char* sublabel = nullptr;           // пояснение под подписью
    float       width    = 0.0f;              // базовые px (до масштаба); 0 — по содержимому
    bool        disabled = false;
    const char* key      = nullptr;           // явная идентичность
    ImVec2      sizePx   = {};                // для контейнеров с посчитанной геометрией: итоговые px, перекрывает width
    const ToggleStyle* style = nullptr;       // nullptr — стиль текущей темы
};

// Переключает value по клику; возвращает true в кадре изменения
bool Toggle(bool& value, const ToggleOptions& options = {});
