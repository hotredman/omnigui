#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include "ui/components/UiKey.hpp"
#include <imgui.h>
#include <optional>
#include <source_location>

// Квадратная плоская кнопка панелей инструментов: иконка или короткий текстовый
// глиф ("Aa", "+") с состоянием «выбрана». Параметры — designated initializers:
//
//     ToolButton({.icon = Icon::Cog, .tooltip = "Settings"});
//     ToolButton({.glyph = "Aa", .tooltip = "Font", .selected = isOn});
//
// Идентичность выводится из key, иначе из места вызова (loc), глифа или иконки.
struct ToolButtonOptions {
    Icon        icon     = Icon::None;
    const char* glyph    = nullptr;                 // текст вместо иконки
    UiVariant   variant  = UiVariant::Default;
    std::optional<UiSize> size;                     // сторона квадрата; не задан — размер из темы
    const char* tooltip  = nullptr;
    bool        selected = false;
    bool        disabled = false;
    ImU32       iconColor = 0;                      // 0 — цвет по состоянию (только для иконки)
    UiKey       key      = {};                      // явная идентичность
    float       sidePx   = 0.0f;                    // для компонентов с посчитанной геометрией: сторона в итоговых px, перекрывает size
};

// Возвращает true в кадре клика
bool ToolButton(const ToolButtonOptions& options,
                std::source_location loc = std::source_location::current());

// Вертикальное выравнивание элемента высотой itemHeightPx по центру панели (Header и т.п.).
// Работает только в окнах высотой с шапку; в обычных окнах ничего не делает.
// allowMoveUp = true — разрешает сдвигать курсор и вверх (нужно после SameLine, когда
// курсор унаследовал Y предыдущего, более низкого элемента)
void ApplyToolbarVerticalCentering(float itemHeightPx, bool allowMoveUp = false);
