#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>

// =========================================================================================
// Компонент переключателя Toggle (современный тумблер-слайдер в стиле @evo/ui/Toggle.svelte)
// =========================================================================================
class Toggle {
public:
    // 1. Рендер переключателя со ссылкой на bool
    static bool Render(const char* id, bool& value,
                       const char* label = nullptr,
                       const char* sublabel = nullptr,
                       bool disabled = false,
                       float width = 0.0f,
                       float height = 0.0f,
                       const ToggleStyle* customStyle = nullptr);

    // 2. Рендер переключателя по указателю на bool
    static bool Render(const char* id, bool* value,
                       const char* label = nullptr,
                       const char* sublabel = nullptr,
                       bool disabled = false,
                       float width = 0.0f,
                       float height = 0.0f,
                       const ToggleStyle* customStyle = nullptr)
    {
        if (!value) return false;
        return Render(id, *value, label, sublabel, disabled, width, height, customStyle);
    }
};
