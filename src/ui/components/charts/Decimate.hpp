#pragma once

#include "ui/components/charts/ChartTypes.hpp"
#include <vector>

namespace ChartCore {

// Min/max-децимация точек для рендеринга по пиксельным колонкам:
// разбивает точки на колонки-корзины и оставляет минимальную и максимальную точки в исходном порядке.
// Гарантирует сохранение пиков и впадин сигнала при любом N без искажений.
std::vector<ChartPoint> DecimateMinMax(const std::vector<ChartPoint>& points, int columns);

} // namespace ChartCore
