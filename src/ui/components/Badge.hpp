#pragma once

#include "ui/components/UiTheme.hpp"

// Параметры семантического чипа/бейджа (designated initializers):
//
//     Badge("RUNNING", {.variant = UiVariant::Success});
//     Badge("beta", {.variant = UiVariant::Warning, .tooltip = "Experimental"});
struct BadgeOptions {
    UiVariant   variant   = UiVariant::Default;
    const char* tooltip   = nullptr;     // подсказка при наведении
    float       topOffset = -1.0f;       // сдвиг вниз от курсора, базовые px; < 0 — без сдвига
    float       fontSize  = 0.0f;        // кегль, pt; 0 — по стилю темы
};

// Универсальный семантический чип/бейдж для отображения статусов, меток и тегов
void Badge(const char* text, const BadgeOptions& options = {});

// Числовой бейдж со значением и опциональной единицей (например: 5 "исп.")
void BadgeNumber(int value, const char* unit = nullptr, const BadgeOptions& options = {});

// Прямая отрисовка бейджа в указанную позицию DrawList (без сдвига ImGui cursor);
// tooltip и topOffset здесь не используются. Возвращает размер бейджа
ImVec2 BadgeDraw(ImDrawList* dl, ImVec2 pos, const char* text, const BadgeOptions& options = {});

// Расчет размера бейджа
ImVec2 BadgeSize(const char* text, const BadgeOptions& options = {});
