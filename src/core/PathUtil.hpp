#pragma once

#include <filesystem>
#include <string>

// UTF-8 строка -> std::filesystem::path (std::filesystem::u8path объявлен устаревшим в C++20).
// Единая точка конвертации: на Windows путь переводится в wchar_t, на остальных ОС остаётся UTF-8.
inline std::filesystem::path PathFromUtf8(const std::string& utf8) {
#if defined(__cpp_char8_t)
    return std::filesystem::path(std::u8string(utf8.begin(), utf8.end()));
#else
    return std::filesystem::u8path(utf8);
#endif
}