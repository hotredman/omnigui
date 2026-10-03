#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <optional>

// Квадратная плоская кнопка панелей инструментов: иконка или короткий текстовый
// глиф ("Aa", "+") с состоянием «выбрана». Параметры — designated initializers:
//
//     ToolButton({.icon = Icon::Cog, .tooltip = "Settings"});
//     ToolButton({.glyph = "Aa", .tooltip = "Font", .selected = isOn});
//
// Идентичность выводится из key, иначе из глифа или иконки.
struct ToolButtonOptions {
    Icon        icon     = Icon::None;
    const char* glyph    = nullptr;                 // текст вместо иконки
    UiVariant   variant  = UiVariant::Default;
    std::optional<UiSize> size;                     // сторона квадрата; не задан — размер из темы
    const char* tooltip  = nullptr;
    bool        selected = false;
    bool        disabled = false;
    ImU32       iconColor = 0;                      // 0 — цвет по состоянию (только для иконки)
    const char* key      = nullptr;                 // явная идентичность
    float       sidePx   = 0.0f;                    // для компонентов с посчитанной геометрией: сторона в итоговых px, перекрывает size
};

// Возвращает true в кадре клика
bool ToolButton(const ToolButtonOptions& options);
