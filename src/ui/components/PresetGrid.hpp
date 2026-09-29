#pragma once

#include "ui/components/UiTheme.hpp"
#include <vector>
#include <string>

class PresetGrid {
public:
    // Базовый рендер селектора пресетов для float
    static bool Render(
        float& value,
        const std::vector<float>& presets,
        const char* unit = nullptr,
        float width = 0.0f,
        int columns = 5,
        bool showDisplay = true,
        float displayHeight = 0.0f,
        const char* displayFormat = "%.1f",
        const char* btnFormat = "%.4g",
        const PresetGridStyle& style = UiTheme::Get().presetGrid
    );

    // Перегрузка рендера селектора пресетов для double
    static bool Render(
        double& value,
        const std::vector<double>& presets,
        const char* unit = nullptr,
        float width = 0.0f,
        int columns = 5,
        bool showDisplay = true,
        float displayHeight = 0.0f,
        const char* displayFormat = "%.1f",
        const char* btnFormat = "%.4g",
        const PresetGridStyle& style = UiTheme::Get().presetGrid
    );
};
