#pragma once

#include <string>

namespace Assets {

// Возвращает абсолютный или канонический путь к ассету, либо пустую строку, если файл не найден.
// Ищет в:
//  - ./<relPath>
//  - ./assets/<relPath>
//  - ../assets/<relPath>
//  - <exe_dir>/assets/<relPath>
//  - <exe_dir>/../assets/<relPath>
//  - <exe_dir>/../../assets/<relPath>
std::string Resolve(const std::string& relPath);

// Проверяет существование ассета
bool Exists(const std::string& relPath);

// Базовый шрифт библиотеки (assets/fonts/Roboto-Regular.ttf); пустая строка, если файл не найден
std::string DefaultFontPath();

} // namespace Assets
