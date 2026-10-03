#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/UiKey.hpp"
#include <cstddef>
#include <string>
#include <vector>
#include <utility>
#include <source_location>

// Параметры панели вкладок (designated initializers):
//
//     TabBar(tabs, selected);
//     TabBar(tabs, selected, {.separator = false, .variants = tabVariants});
struct TabBarOptions {
    UiSize             size      = UiSize::Medium;  // высота вкладок по шкале UiSize
    bool               separator = true;      // линия-разделитель под панелью
    const UiVariant*   variants  = nullptr;   // по вкладке; nullptr — все обычные
    UiKey              key       = {};        // идентичность; по умолчанию — подпись первой вкладки
    const TabBarStyle* style     = nullptr;   // оверрайд стиля; nullptr — из темы
};

// 1. Панель вкладок из C-массива строк. Возвращает true, если выбрана другая вкладка
bool TabBar(const char* const items[], int itemsCount, int& selectedIndex,
            const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current());

// 2. То же из массива, размер выводится автоматически
template<std::size_t N>
bool TabBar(const char* const (&items)[N], int& selectedIndex, const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current()) {
    return TabBar(items, static_cast<int>(N), selectedIndex, options, loc);
}

// 3. Из вектора строк
bool TabBar(const std::vector<std::string>& items, int& selectedIndex,
            const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current());

// 4. Для любых enum-типов: вкладки — пары (значение, подпись)
template<typename EnumT>
bool TabBar(const std::vector<std::pair<EnumT, std::string>>& items, EnumT& currentItem,
            const TabBarOptions& options = {},
            std::source_location loc = std::source_location::current())
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
    bool changed = TabBar(labels, selectedIndex, options, loc);
    if (changed && selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size())) {
        currentItem = items[selectedIndex].first;
    }
    return changed;
}

struct TabItemOptions {
    UiSize             size     = UiSize::Medium;      // высота вкладки по шкале UiSize
    UiVariant          variant  = UiVariant::Default;  // не Default — цвет и подчёркивание вкладки
    bool               sameLine = false;               // продолжить строку (с зазором из стиля)
    UiKey              key      = {};                  // идентичность; по умолчанию — label
    const TabBarStyle* style    = nullptr;             // оверрайд стиля; nullptr — из темы

    // Внутренний механизм: финальные px; 0 — авто (по тексту / высота стиля)
    ImVec2             sizePx   = ImVec2(0.0f, 0.0f);
};

// Одиночный элемент вкладки (для собственных раскладок; обычно хватает TabBar)
bool TabItem(const char* label, bool selected, const TabItemOptions& options = {},
             std::source_location loc = std::source_location::current());

// Горизонтальная разделительная линия под панелью вкладок
void TabSeparator(const TabBarStyle* style = nullptr);
