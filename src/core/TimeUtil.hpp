#pragma once

#include <ctime>

// Потокобезопасное локальное время: localtime_s (Windows) / localtime_r (POSIX).
// Единственное место с ветвлением по ОС для этой задачи.
inline bool LocalTime(std::time_t time, std::tm& out) {
#if defined(_WIN32)
    return localtime_s(&out, &time) == 0;
#else
    return localtime_r(&time, &out) != nullptr;
#endif
}