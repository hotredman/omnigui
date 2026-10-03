#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <source_location>

// =========================================================================================
// Компонент переключателя Toggle (современный тумблер-слайдер в стиле @evo/ui/Toggle.svelte)
// =========================================================================================

// Параметры переключателя (designated initializers):
//
//     Toggle(enabled, {.label = "Auto", .sublabel = "Adjusts on its own"});
//
// Идентичность выводится из key, иначе из места вызова (loc) и подписи.
struct ToggleOptions {
    const char* label    = nullptr;
    UiSize      size     = UiSize::Medium;    // масштаб по шкале UiSize (Medium — стиль темы)
    const char* sublabel = nullptr;           // пояснение под подписью
    float       width    = 0.0f;              // базовые px (до масштаба); 0 — по содержимому
    bool        disabled = false;
    UiKey       key      = {};                // явная идентичность
    ImVec2      sizePx   = {};                // для контейнеров с посчитанной геометрией: итоговые px, перекрывает width
    const ToggleStyle* style = nullptr;       // nullptr — стиль текущей темы
};

// Переключает value по клику; возвращает true в кадре изменения
bool Toggle(bool& value, const ToggleOptions& options = {},
            std::source_location loc = std::source_location::current());
