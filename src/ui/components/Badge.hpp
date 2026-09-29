#pragma once

#include "ui/components/UiTheme.hpp"

// Универсальный семантический чип/бейдж для отображения статусов, меток и тегов
class Badge {
public:
    // Отрисовка текстового бейджа (например: «Свой», «Своё выражение», «Дубликат символа», «В методике»)
    static void Render(const char* text, UiVariant variant = UiVariant::Default, float topOffset = -1.0f, float fontSize = 0.0f, const char* tooltip = nullptr);

    // Прямая отрисовка бейджа в указанную позицию DrawList (без сдвига ImGui cursor)
    static ImVec2 Draw(ImDrawList* dl, ImVec2 pos, const char* text, UiVariant variant = UiVariant::Default, float fontSize = 0.0f);

    // Расчет размера бейджа
    static ImVec2 CalcSize(const char* text, float fontSize = 0.0f);

    // Отрисовка числового бейджа со значением и опциональной единицей (например: 5 "исп.")
    static void Number(int value, const char* unit = nullptr, UiVariant variant = UiVariant::Default, float topOffset = -1.0f, float fontSize = 0.0f);
};
