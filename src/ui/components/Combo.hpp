#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/UiKey.hpp"
#include <imgui.h>
#include <string>
#include <vector>
#include <type_traits>
#include <source_location>

// Параметры выпадающего списка (designated initializers):
//
//     Combo(mode, modes, 3, {.label = "Mode", .width = 200.0f});
//
// Идентичность выводится из key, иначе из места вызова (loc) и подписи.
struct ComboOptions {
    const char* label   = nullptr;              // подпись над списком
    UiVariant   variant = UiVariant::Default;
    UiSize      size    = UiSize::Medium;
    float       width   = 0.0f;                 // базовые px (до масштаба); 0 — по содержимому; Fill — на всю ширину
    UiKey       key     = {};                   // явная идентичность
    ImVec2      sizePx  = {};                   // для контейнеров с посчитанной геометрией: итоговые px, перекрывают width и size
    const ComboStyle* style = nullptr;          // nullptr — стиль текущей темы
};

// Возвращает true в кадре выбора нового пункта
bool Combo(int& currentItem, const char* const items[], int itemsCount, const ComboOptions& options = {},
           std::source_location loc = std::source_location::current());
bool Combo(int& currentItem, const std::vector<std::string>& items, const ComboOptions& options = {},
           std::source_location loc = std::source_location::current());

// Строго типизированный enum + массив с автовыводом размера
template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
bool Combo(EnumT& currentItem, const char* const (&items)[N], const ComboOptions& options = {},
           std::source_location loc = std::source_location::current()) {
    int current = static_cast<int>(currentItem);
    if (Combo(current, items, static_cast<int>(N), options, loc)) {
        currentItem = static_cast<EnumT>(current);
        return true;
    }
    return false;
}
