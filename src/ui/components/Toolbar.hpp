#pragma once

#include "ui/components/Bar.hpp"

// Панель инструментов под Header (Toolbar): зоны Left / Center(Fill) / Right.
// RAII-область: конструктор открывает окно, деструктор закрывает.
//
//     if (auto toolbar = Toolbar()) {
//         if (auto left = toolbar.Left()) { left.Label("Session:"); }
//     }
class Toolbar : public Bar {
public:
    // posY < 0 — сразу под Header (theme.HeaderHeight());
    // height <= 0 — высота из стиля темы; customStyle == nullptr — стиль текущей темы
    explicit Toolbar(float posY = -1.0f, float height = 0.0f,
                     const ToolbarStyle* customStyle = nullptr);

    // Центральная зона, занимающая остаток ширины (синоним Center)
    CenterZone Fill() { return Center(); }
};
