#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>
#include <vector>
#include <type_traits>

class Combo {
public:
    // 1. Автономный рендер без лейбла с поддержкой UiSize и автоподгоном ширины по содержимому (width <= 0.0f)
    static bool Render(const char* id, int& currentItem, 
                       const char* const items[], int itemsCount, 
                       UiSize size = UiSize::Medium,
                       float width = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    // 2. Рендер с лейблом (для карточек и форм ввода)
    static bool Render(const char* id, const char* label, int& currentItem, 
                       const char* const items[], int itemsCount, 
                       UiSize size = UiSize::Medium,
                       float width = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    // 3. Перегрузки для явного указания float width / float height (обратная совместимость)
    static bool Render(const char* id, int& currentItem, 
                       const char* const items[], int itemsCount, 
                       float width, float height = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    static bool Render(const char* id, const char* label, int& currentItem, 
                       const char* const items[], int itemsCount, 
                       float width, float height = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    // 4. Перегрузки для std::vector<std::string> с UiSize
    static bool Render(const char* id, int& currentItem, 
                       const std::vector<std::string>& items, 
                       UiSize size = UiSize::Medium,
                       float width = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    static bool Render(const char* id, const char* label, int& currentItem, 
                       const std::vector<std::string>& items, 
                       UiSize size = UiSize::Medium,
                       float width = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    static bool Render(const char* id, int& currentItem, 
                       const std::vector<std::string>& items, 
                       float width, float height = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    static bool Render(const char* id, const char* label, int& currentItem, 
                       const std::vector<std::string>& items, 
                       float width, float height = 0.0f, 
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr);

    // 5. Шаблонные методы для строго типизированных Enum и массивов с автовыводом размера
    template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    static bool Render(const char* id, EnumT& currentItem, 
                       const char* const (&items)[N], 
                       UiSize size = UiSize::Medium,
                       float width = 0.0f,
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr)
    {
        int current = static_cast<int>(currentItem);
        if (Render(id, current, items, static_cast<int>(N), size, width, variant, customStyle)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }

    template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    static bool Render(const char* id, const char* label, EnumT& currentItem, 
                       const char* const (&items)[N], 
                       UiSize size = UiSize::Medium,
                       float width = 0.0f,
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr)
    {
        int current = static_cast<int>(currentItem);
        if (Render(id, label, current, items, static_cast<int>(N), size, width, variant, customStyle)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }

    template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    static bool Render(const char* id, EnumT& currentItem, 
                       const char* const (&items)[N], 
                       float width, float height = 0.0f,
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr)
    {
        int current = static_cast<int>(currentItem);
        if (Render(id, current, items, static_cast<int>(N), width, height, variant, customStyle)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }

    template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
    static bool Render(const char* id, const char* label, EnumT& currentItem, 
                       const char* const (&items)[N], 
                       float width, float height = 0.0f,
                       UiVariant variant = UiVariant::Default, 
                       const ComboStyle* customStyle = nullptr)
    {
        int current = static_cast<int>(currentItem);
        if (Render(id, label, current, items, static_cast<int>(N), width, height, variant, customStyle)) {
            currentItem = static_cast<EnumT>(current);
            return true;
        }
        return false;
    }
};
