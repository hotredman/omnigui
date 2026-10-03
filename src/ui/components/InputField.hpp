#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/TextSuggest.hpp"
#include "ui/components/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <optional>
#include <source_location>

// Параметры поля ввода (designated initializers):
//
//     InputField(rate, {.label = "Rate", .unit = "MB/s"});
//     InputField(name, {.label = "Name", .hint = "Enter a name"});
//
// Идентичность выводится из key, иначе из места вызова (loc) и подписи.
struct InputFieldOptions {
    const char* label   = nullptr;              // подпись над полем
    const char* unit    = nullptr;              // единица измерения внутри поля справа
    const char* format  = "%.2f";               // printf-формат для float/double
    const char* hint    = " — ";                // подсказка пустого поля (текст, optional)
    UiVariant   variant = UiVariant::Default;   // цвет рамки (например, Warning для изменённого значения)
    UiSize      size    = UiSize::Medium;       // высота поля
    float       width   = 0.0f;                 // базовые px (до масштаба); 0 — ширина по умолчанию; Fill — на всю ширину
    UiKey       key     = {};                   // явная идентичность
    ImVec2      sizePx  = {};                   // для контейнеров с посчитанной геометрией: итоговые px, перекрывают width и size
    const TextSuggest* suggest = nullptr;       // подсказки под полем (только для std::string); список строит вызывающий
};

// Все перегрузки возвращают true в кадре изменения значения.
// Число с плавающей точкой и встроенной единицей измерения
bool InputField(float& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
bool InputField(double& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
// Целое число
bool InputField(int& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
// Текстовое поле с placeholder (options.hint)
bool InputField(std::string& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
// Опциональные числа (nullopt показывается как options.hint)
bool InputField(std::optional<float>& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
bool InputField(std::optional<double>& value, const InputFieldOptions& options = {},
                std::source_location loc = std::source_location::current());
