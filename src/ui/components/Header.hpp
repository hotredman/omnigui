#pragma once

#include "ui/components/Bar.hpp"

// Верхняя полоса окна (Header) во всю ширину: зоны Left / Center / Right.
// RAII-область: конструктор открывает окно, деструктор закрывает.
//
//     if (auto header = Header()) {
//         if (auto left = header.Left()) { ... }
//     }
class Header : public Bar {
public:
    // height <= 0 — высота из стиля темы; customStyle == nullptr — стиль текущей темы
    explicit Header(float height = 0.0f, const HeaderStyle* customStyle = nullptr);
};
