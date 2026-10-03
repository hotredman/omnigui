#pragma once

#include "ui/components/Scope.hpp"
#include <imgui.h>

// Область идентичности виджетов (обёртка над стеком ImGui ID). Нужна, когда
// одинаковые виджеты повторяются — в цикле, в строках таблицы, в списке:
//
//     for (auto& run : runs) {
//         if (IdScope id(run.id.c_str())) { Button("Delete"); }
//     }
//
// Конструктор кладёт идентификатор в стек, деструктор снимает. Область всегда
// «открыта» (operator bool == true), чтобы её можно было писать в if-инициализаторе.
class IdScope : public Scope {
public:
    explicit IdScope(const char* key) { ImGui::PushID(key); m_open = true; }
    explicit IdScope(int index)       { ImGui::PushID(index); m_open = true; }
    explicit IdScope(const void* ptr) { ImGui::PushID(ptr); m_open = true; }
    ~IdScope() { ImGui::PopID(); }
};
