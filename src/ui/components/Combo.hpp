#pragma once

#include "ui/components/UiTheme.hpp"
#include <imgui.h>
#include <string>
#include <vector>
#include <type_traits>

// Параметры выпадающего списка (designated initializers):
//
//     Combo(mode, modes, 3, {.label = "Mode", .width = 200.0f});
//
// Идентичность выводится из подписи; без подписи (или для одинаковых подписей)
// задаётся key либо область IdScope.
struct ComboOptions {
    // Ширина на всю доступную ширину контейнера
    static constexpr float Fill = -1.0f;

    const char* label   = nullptr;              // подпись над списком
    UiVariant   variant = UiVariant::Default;
    UiSize      size    = UiSize::Medium;
    float       width   = 0.0f;                 // базовые px (до масштаба); 0 — по содержимому; Fill — на всю ширину
    const char* key     = nullptr;              // явная идентичность
    ImVec2      sizePx  = {};                   // для контейнеров с посчитанной геометрией: итоговые px, перекрывают width и size
    const ComboStyle* style = nullptr;          // nullptr — стиль текущей темы
};

// Возвращает true в кадре выбора нового пункта
bool Combo(int& currentItem, const char* const items[], int itemsCount, const ComboOptions& options = {});
bool Combo(int& currentItem, const std::vector<std::string>& items, const ComboOptions& options = {});

// Строго типизированный enum + массив с автовыводом размера
template<typename EnumT, size_t N, std::enable_if_t<std::is_enum_v<EnumT>, int> = 0>
bool Combo(EnumT& currentItem, const char* const (&items)[N], const ComboOptions& options = {}) {
    int current = static_cast<int>(currentItem);
    if (Combo(current, items, static_cast<int>(N), options)) {
        currentItem = static_cast<EnumT>(current);
        return true;
    }
    return false;
}
