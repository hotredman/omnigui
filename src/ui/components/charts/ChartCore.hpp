#pragma once

#include "ui/components/charts/ChartTypes.hpp"
#include <vector>
#include <string>

namespace ChartCore {

// Генерация «красивых» шагов сетки (1/2/5 x 10^n), выровненных по сетке
std::vector<double> GenerateTicks(double min, double max, int count = 5);

// Генерация промежуточных тиков для минорной сетки (в 5 раз чаще)
std::vector<double> MinorTicks(double min, double max, int majorCount = 5);

// Форматирование числового значения метки оси
std::string FormatTickValue(double val);

// Проекция координат между пространством данных и экранными пикселями
struct Projection {
    ChartBounds bounds;
    ChartPad pad;
    ImVec2 origin{ 0.0f, 0.0f };
    ImVec2 canvasSize{ 0.0f, 0.0f };
    ImVec2 plotMin{ 0.0f, 0.0f };
    ImVec2 plotMax{ 0.0f, 0.0f };
    float plotWidth = 1.0f;
    float plotHeight = 1.0f;

    static Projection Create(const ChartBounds& bounds, const ChartPad& pad, ImVec2 origin, ImVec2 canvasSize);

    float ToScreenX(double x) const;
    float ToScreenY(double y) const;
    ImVec2 ToScreen(const ChartPoint& pt) const;

    double ToDataX(float screenX) const;
    double ToDataY(float screenY) const;
    ChartPoint ToData(const ImVec2& screenPt) const;
};

// Бинарный поиск ближайшей точки по координате X за O(log N)
int NearestIndexByX(const std::vector<ChartPoint>& points, double targetX);

// Отсечение прямой линии по прямоугольнику [xmin, xmax] x [ymin, ymax] алгоритмом Liang-Barsky
bool ClipInfiniteLineToRect(double x1, double y1, double x2, double y2,
                            double xmin, double ymin, double xmax, double ymax,
                            double& outX1, double& outY1, double& outX2, double& outY2);

} // namespace ChartCore
