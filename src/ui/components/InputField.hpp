#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <string>
#include <optional>

// Параметры поля ввода (designated initializers):
//
//     InputField(rate, {.label = "Rate", .unit = "MB/s"});
//     InputField(name, {.label = "Name", .hint = "Enter a name"});
//
// Идентичность выводится из подписи; без подписи (или для одинаковых подписей)
// задаётся key либо область IdScope.
struct InputOptions {
    // Ширина на всю доступную ширину контейнера
    static constexpr float Fill = -1.0f;

    const char* label   = nullptr;              // подпись над полем
    const char* unit    = nullptr;              // единица измерения внутри поля справа
    const char* format  = "%.2f";               // printf-формат для float/double
    const char* hint    = " — ";                // подсказка пустого поля (текст, optional)
    UiVariant   variant = UiVariant::Default;   // цвет рамки (например, Warning для изменённого значения)
    UiSize      size    = UiSize::Medium;       // высота поля
    float       width   = 0.0f;                 // базовые px (до масштаба); 0 — ширина по умолчанию; Fill — на всю ширину
    const char* key     = nullptr;              // явная идентичность
    ImVec2      sizePx  = {};                   // для контейнеров с посчитанной геометрией: итоговые px, перекрывают width и size
};

// Все перегрузки возвращают true в кадре изменения значения.
// Число с плавающей точкой и встроенной единицей измерения
bool InputField(float& value, const InputOptions& options = {});
bool InputField(double& value, const InputOptions& options = {});
// Целое число
bool InputField(int& value, const InputOptions& options = {});
// Текстовое поле с placeholder (options.hint)
bool InputField(std::string& value, const InputOptions& options = {});
// Опциональные числа (nullopt показывается как options.hint)
bool InputField(std::optional<float>& value, const InputOptions& options = {});
bool InputField(std::optional<double>& value, const InputOptions& options = {});
