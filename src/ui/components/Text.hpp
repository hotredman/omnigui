#pragma once

#include "ui/components/UiTheme.hpp"
#include "ui/components/Icon.hpp"
#include <imgui.h>
#include <optional>
#include <string>

// Начертание текста: определяет шрифт и цвет по умолчанию
enum class TextRole {
    Body,   // обычный текст
    Title,  // заголовок (жирный)
    Muted   // приглушённый вспомогательный текст
};

// Параметры текста (designated initializers):
//
//     Text("Connected", {.variant = UiVariant::Success, .icon = Icon::Check});
//     Text("Section", {.role = TextRole::Title});
//     Text(longDescription, {.variant = UiVariant::Secondary, .wrap = true});
struct TextOptions {
    UiVariant             variant = UiVariant::Default;   // семантический цвет (у Muted не используется)
    TextRole              role    = TextRole::Body;
    Icon                  icon    = Icon::None;           // иконка слева от текста
    std::optional<UiSize> size;                           // кегль по метрикам UiSize; не задан — по умолчанию
    bool                  wrap    = false;                // многострочный текст с переносом по границе контейнера (без иконки)
};

// Выводит строку текста в текущую позицию раскладки
void Text(const char* text, const TextOptions& options = {});
void Text(const std::string& text, const TextOptions& options = {});

// printf-форматирование в std::string: Text(Format("%.1f MB/s", v), {.role = TextRole::Muted})
#if defined(__GNUC__)
__attribute__((format(printf, 1, 2)))
#endif
std::string Format(const char* format, ...);
