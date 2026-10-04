#pragma once

#include "omnikit/UiTheme.hpp"
#include "omnikit/UiKey.hpp"
#include <string>
#include <source_location>

namespace OmniKit {

// Параметры поля поиска (designated initializers):
//
//     SearchInput(query, {.hint = "Search datasets..."});
//     SearchInput(query, {.hint = "Filter...", .width = 220});
//     SearchInput(query, {.hint = "Search...", .width = Fill});   // на всю ширину
struct SearchInputOptions {
    const char* hint  = "Search...";       // плейсхолдер
    UiSize      size  = UiSize::Small;
    float       width = 0.0f;              // базовые px (масштабируются внутри); 0 — ширина по умолчанию; Fill — вся ширина
    UiKey       key   = {};                // идентичность; по умолчанию — hint
};

// Поле поиска дизайн-системы с поддержкой UiSize, плейсхолдера и кнопки быстрой очистки.
// Возвращает true, если строка запроса изменилась
bool SearchInput(std::string& query, const SearchInputOptions& options = {},
                 std::source_location loc = std::source_location::current());

} // namespace OmniKit
