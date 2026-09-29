#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <cstdint>

enum class LineStyle {
    Solid,  // Сплошная линия
    Dashed, // Штриховая линия
    Dotted  // Пунктирная линия
};

enum class MarkerShape {
    Diamond,  // ◆ Ромб
    Circle,   // ● Круг
    Square,   // ■ Квадрат
    Triangle, // ▲ Треугольник
    Cross     // ✖ Крестик
};

struct ChartPoint {
    double x = 0.0;
    double y = 0.0;

    ChartPoint() = default;
    constexpr ChartPoint(double x_, double y_) : x(x_), y(y_) {}
};

struct ChartBounds {
    double minX = 0.0;
    double maxX = 1.0;
    double minY = 0.0;
    double maxY = 1.0;

    bool IsValid() const {
        return maxX > minX && maxY > minY;
    }
};

struct ChartPad {
    float top = 30.0f;
    float right = 30.0f;
    float bottom = 56.0f;
    float left = 75.0f;
};

struct ChartOptions {
    bool majorGrid   = true; // Основная сетка по крупным делениям осей
    bool minorGrid   = true; // Вспомогательная подсетка (x5)
    bool crosshair   = true; // Перекрестие зонда при наведении курсора
    bool stickyZero  = true; // Умный зум с фиксацией X=0 в упругой зоне
    bool panEnabled  = true; // Разрешено ли панорамирование зажатием ЛКМ
    bool zoomEnabled = true; // Разрешен ли зум колесом мыши
};

// ============================================================================
// Серия данных (кривая)
// ============================================================================
struct ChartSeries {
    std::string id;
    std::string label;
    std::string sublabel;
    std::vector<ChartPoint> points;

    ImU32 color = 0;  // 0 — кривая темы (ChartStyle::colLine)
    float thickness = 2.0f;
    LineStyle style = LineStyle::Solid;
};

// ============================================================================
// Вспомогательная линия (хорда упругости, параллель, порог)
// ============================================================================
struct ChartLine {
    std::string id;
    std::string label;
    std::string sublabel;
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;

    ImU32 color = 0;  // 0 — кривая темы (ChartStyle::colLine)
    float thickness = 1.5f;
    LineStyle style = LineStyle::Dashed;
    bool infinite = false;
    float labelPosition = 0.5f; // 0.0 .. 1.0 вдоль отрезка
};

// ============================================================================
// Маркер характерной точки (пик, предел текучести, точка разрыва)
// ============================================================================
struct ChartMarker {
    std::string id;
    std::string label;
    std::string sublabel;
    double x = 0.0;
    double y = 0.0;

    ImU32 color = 0;  // 0 — кривая темы (ChartStyle::colLine)
    float size = 6.0f;
    MarkerShape shape = MarkerShape::Diamond;
    bool projectX = true; // Пунктирная проекция на ось X (вниз)
    bool projectY = true; // Пунктирная проекция на ось Y (влево)
};

// ============================================================================
// Событие наведения курсора (зонд)
// ============================================================================
struct ChartHoverInfo {
    bool hasHover = false;
    int seriesIndex = -1;
    size_t pointIndex = 0;
    double dataX = 0.0;
    double dataY = 0.0;
    ImVec2 screenPos{ 0.0f, 0.0f };
};
