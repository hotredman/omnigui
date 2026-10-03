#pragma once

#include "ui/components/Bar.hpp"

// Верхняя полоса окна (Header) во всю ширину: зоны Left / Center / Right.
// RAII-область: конструктор открывает окно, деструктор закрывает.
//
//     if (auto header = Header()) {
//         if (auto left = header.Left()) { ... }
//     }
// Параметры шапки (designated initializers):
//
//     Header({.heightPx = 64});
struct HeaderOptions {
    float              heightPx = 0.0f;      // итоговые px; <= 0 — высота из стиля темы
    const HeaderStyle* style    = nullptr;   // оверрайд стиля; nullptr — из темы
};

class Header : public Bar {
public:
    explicit Header(const HeaderOptions& options = {});
};
