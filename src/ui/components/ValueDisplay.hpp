#pragma once

#include "ui/components/UiTheme.hpp"

class ValueDisplay {
public:
    // Отрисовка числового значения с единицей измерения и форматированием
    static void Render(double value, const char* unit = nullptr, 
                       float width = 0.0f, float height = 0.0f, 
                       const char* format = "%.2f",
                       const ValueDisplayStyle* customStyle = nullptr);

    // Отрисовка строкового значения с единицей измерения
    static void Render(const char* text, const char* unit = nullptr, 
                       float width = 0.0f, float height = 0.0f,
                       const ValueDisplayStyle* customStyle = nullptr);
};
