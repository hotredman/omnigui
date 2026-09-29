#pragma once

#include "ui/components/UiTheme.hpp"
#include <string>

// Поле поиска дизайн-системы с поддержкой UiSize, плейсхолдера и кнопки быстрой очистки
class SearchInput {
public:
    static bool Render(const char* id,
                       std::string& query,
                       const char* hint = "Search...",
                       float baseWidth = 0.0f,
                       UiSize size = UiSize::Small);
};
