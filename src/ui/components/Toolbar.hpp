#pragma once

#include "ui/components/Bar.hpp"

// Панель инструментов под Header (Toolbar): зоны Left / Center(Fill) / Right.
// RAII-область: конструктор открывает окно, деструктор закрывает.
//
//     if (auto toolbar = Toolbar()) {
//         if (auto left = toolbar.Left()) { left.Label("Session:"); }
//     }
// Параметры панели инструментов (designated initializers):
//
//     Toolbar({.heightPx = 48});
struct ToolbarOptions {
    float               posYPx   = -1.0f;    // итоговые px; < 0 — сразу под Header
    float               heightPx = 0.0f;     // итоговые px; <= 0 — высота из стиля темы
    const ToolbarStyle* style    = nullptr;  // оверрайд стиля; nullptr — из темы
};

class Toolbar : public Bar {
public:
    explicit Toolbar(const ToolbarOptions& options = {});

    // Центральная зона, занимающая остаток ширины (синоним Center)
    CenterZone Fill() { return Center(); }
};
