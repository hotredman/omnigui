#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Combo.hpp"
#include <string>
#include <vector>
#include <optional>
#include <type_traits>

class InputField {
public:
    // Поле ввода числа с плавающей точкой и встроенной единицей измерения
    static bool Float(const char* id, const char* label, float& value, 
                      const char* unit = nullptr, float width = 0.0f, const char* format = "%.2f",
                      float height = 0.0f, UiVariant variant = UiVariant::Default);

    // Опциональное поле ввода числа с плавающей точкой (nullopt -> " — ")
    static bool Float(const char* id, const char* label, std::optional<float>& value, 
                      const char* unit = nullptr, float width = 0.0f, const char* format = "%.2f",
                      float height = 0.0f, UiVariant variant = UiVariant::Default);

    // Поле ввода числа двойной точности с плавающей точкой и встроенной единицей измерения
    static bool Double(const char* id, const char* label, double& value, 
                       const char* unit = nullptr, float width = 0.0f, const char* format = "%.2f",
                       float height = 0.0f, UiVariant variant = UiVariant::Default);

    // Опциональное поле ввода числа двойной точности (nullopt -> " — ")
    static bool Double(const char* id, const char* label, std::optional<double>& value, 
                       const char* unit = nullptr, float width = 0.0f, const char* format = "%.2f",
                       float height = 0.0f, UiVariant variant = UiVariant::Default);

    // Поле ввода целого числа
    static bool Int(const char* id, const char* label, int& value, 
                    const char* unit = nullptr, float width = 0.0f,
                    float height = 0.0f, UiVariant variant = UiVariant::Default);

    // Текстовое поле ввода с поддержкой placeholder/hint (по умолчанию " — ")
    static bool Text(const char* id, const char* label, std::string& value, 
                     const char* hint = " — ", float width = 0.0f,
                     float height = 0.0f, UiVariant variant = UiVariant::Default);

    // Выпадающий список (Combo)
    static bool Combo(const char* id, const char* label, int& currentItem, 
                      const char* const items[], int itemsCount, float width = 0.0f,
                      float height = 0.0f, UiVariant variant = UiVariant::Default);

    static bool Combo(const char* id, const char* label, int& currentItem, 
                      const std::vector<std::string>& items, float width = 0.0f,
                      float height = 0.0f, UiVariant variant = UiVariant::Default);

    // Шаблонный Combo для строго типизированных Enum и массивов с автовыводом размера
    template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    static bool Combo(const char* id, const char* label, EnumT& currentItem, 
                      const char* const (&items)[N], float width = 0.0f,
                      float height = 0.0f, UiVariant variant = UiVariant::Default)
    {
        int current = static_cast<int>(currentItem);
        if (Combo(id, label, current, items, static_cast<int>(N), width, height, variant)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }
};
