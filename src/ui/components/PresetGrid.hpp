#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>
#include <vector>

// Параметры сетки пресетов (designated initializers):
//
//     PresetGrid(rate, {10.0f, 50.0f, 100.0f}, {.unit = "Hz", .columns = 3});
struct PresetGridOptions {
    const char* unit          = nullptr;   // единица измерения на табло
    int         columns       = 5;
    bool        showDisplay   = true;      // табло текущего значения над сеткой
    const char* displayFormat = "%.1f";    // формат значения на табло
    const char* buttonFormat  = "%.4g";    // формат подписи кнопки пресета
    const char* key           = nullptr;   // идентичность; по умолчанию — адрес значения
    const PresetGridStyle* style = nullptr;  // оверрайд стиля; nullptr — из темы

    // Внутренний механизм для контейнеров (Card), которые сами считают геометрию:
    // финальные px (ширина, высота табло); 0 — авто. Не для кода приложения.
    ImVec2      sizePx        = ImVec2(0.0f, 0.0f);
};

// Селектор пресетов: табло значения + сетка кнопок. Возвращает true, если значение изменилось
bool PresetGrid(float& value, const std::vector<float>& presets, const PresetGridOptions& options = {});
bool PresetGrid(double& value, const std::vector<double>& presets, const PresetGridOptions& options = {});
