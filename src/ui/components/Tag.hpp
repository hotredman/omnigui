#pragma once

#include "ui/components/UiTheme.hpp"
#include <optional>
#include <string>
#include <vector>

// Параметры тега-чипа (designated initializers):
//
//     if (Tag("vibration")) { ... }                       // кликабельный "# vibration"
//     Tag("readonly", {.clickable = false});
struct TagOptions {
    const char* prefix    = "#";       // префикс перед текстом; nullptr или "" — без префикса
    bool        clickable = true;
    const char* tooltip   = nullptr;   // подсказка; nullptr — "Filter by tag #<text>"
};

// Интерактивный или информационный тег-чип в стиле evo-machine-cs (# тег)
// Возвращает true, если тег был нажат пользователем
bool Tag(const char* text, const TagOptions& options = {});

struct TagListOptions {
    float maxWidthPx = 0.0f;   // ширина переноса в финальных px; 0 — вся доступная ширина
};

// Набор тегов с автоматическим переносом по доступной ширине.
// Возвращает нажатый тег (или std::nullopt, если ничего не нажато)
std::optional<std::string> TagList(const std::vector<std::string>& tags,
                                   const TagListOptions& options = {});
