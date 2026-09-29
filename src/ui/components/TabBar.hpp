#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>
#include <vector>
#include <utility>

class TabBar {
public:
    // 1. Рендеринг панели вкладок из C-массива строк
    static bool Render(const char* id,
                       const char* const items[],
                       int itemsCount,
                       int& selectedIndex,
                       bool showSeparator = true,
                       float itemWidth = 0.0f,
                       float itemHeight = 0.0f,
                       const TabBarStyle* customStyle = nullptr,
                       const UiVariant* variants = nullptr);  // по вкладке; nullptr — все обычные

    // 2. Рендеринг панели вкладок из вектора строк
    static bool Render(const char* id,
                       const std::vector<std::string>& items,
                       int& selectedIndex,
                       bool showSeparator = true,
                       float itemWidth = 0.0f,
                       float itemHeight = 0.0f,
                       const TabBarStyle* customStyle = nullptr);

    // 3. Шаблонный рендеринг для любых enum-типов
    template<typename EnumT>
    static bool Render(const char* id,
                       const std::vector<std::pair<EnumT, std::string>>& items,
                       EnumT& currentItem,
                       bool showSeparator = true,
                       float itemWidth = 0.0f,
                       float itemHeight = 0.0f,
                       const TabBarStyle* customStyle = nullptr)
    {
        int selectedIndex = -1;
        for (size_t i = 0; i < items.size(); ++i) {
            if (currentItem == items[i].first) {
                selectedIndex = static_cast<int>(i);
                break;
            }
        }
        std::vector<std::string> labels;
        labels.reserve(items.size());
        for (const auto& it : items) {
            labels.push_back(it.second);
        }
        bool changed = Render(id, labels, selectedIndex, showSeparator, itemWidth, itemHeight, customStyle);
        if (changed && selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size())) {
            currentItem = items[selectedIndex].first;
        }
        return changed;
    }

    // 4. Одиночный элемент вкладки
    static bool TabItem(const char* id,
                        const char* label,
                        bool isSelected,
                        float width = 0.0f,
                        float height = 0.0f,
                        bool sameLine = false,
                        const TabBarStyle* customStyle = nullptr,
                        UiVariant variant = UiVariant::Default);  // не Default — цвет и подчёркивание вкладки

    // 5. Горизонтальная разделительная линия под таббаром
    static void AddSeparator(const TabBarStyle* customStyle = nullptr);

    // 6. Опциональный оверрайд стиля для единичного таббара
    static bool RenderEx(const char* id,
                         const char* const items[],
                         int itemsCount,
                         int& selectedIndex,
                         const TabBarStyle& customStyle,
                         bool showSeparator = true,
                         float itemWidth = 0.0f,
                         float itemHeight = 0.0f);
};
