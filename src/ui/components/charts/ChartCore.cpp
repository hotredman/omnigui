#include "ui/components/charts/ChartCore.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace ChartCore {

std::vector<double> GenerateTicks(double min, double max, int count) {
    if (count <= 0) count = 5;
    double span = max - min;
    if (span <= 0.0) {
        return { min };
    }

    double rawStep = span / count;
    double mag = std::pow(10.0, std::floor(std::log10(rawStep)));
    double step = mag;
    if (rawStep / mag >= 5.0) {
        step = 5.0 * mag;
    } else if (rawStep / mag >= 2.0) {
        step = 2.0 * mag;
    }

    std::vector<double> ticks;
    double start = std::ceil(min / step) * step;
    double eps = step * 1e-6;

    for (double v = start; v <= max + step * 0.01; v += step) {
        if (std::abs(v) < eps) {
            v = 0.0;
        }
        ticks.push_back(v);
    }

    if (ticks.empty()) {
        ticks.push_back(min);
    }
    return ticks;
}

std::vector<double> MinorTicks(double min, double max, int majorCount) {
    return GenerateTicks(min, max, majorCount * 5);
}

std::string FormatTickValue(double val) {
    if (std::abs(val) < 1e-12) {
        val = 0.0;
    }

    char buf[64];
    double absVal = std::abs(val);
    if (absVal >= 10000.0 || (absVal > 0.0 && absVal < 0.01)) {
        std::snprintf(buf, sizeof(buf), "%.1e", val);
        return std::string(buf);
    }

    if (std::floor(val) == val) {
        std::snprintf(buf, sizeof(buf), "%.0f", val);
        return std::string(buf);
    }

    if (absVal < 1.0) {
        std::snprintf(buf, sizeof(buf), "%.2f", val);
    } else {
        std::snprintf(buf, sizeof(buf), "%.1f", val);
    }
    return std::string(buf);
}

Projection Projection::Create(const ChartBounds& bounds, const ChartPad& pad, ImVec2 origin, ImVec2 canvasSize) {
    Projection p;
    p.bounds = bounds;
    p.pad = pad;
    p.origin = origin;
    p.canvasSize = canvasSize;

    p.plotMin = ImVec2(origin.x + pad.left, origin.y + pad.top);
    p.plotMax = ImVec2(origin.x + canvasSize.x - pad.right, origin.y + canvasSize.y - pad.bottom);
    p.plotWidth = std::max(10.0f, p.plotMax.x - p.plotMin.x);
    p.plotHeight = std::max(10.0f, p.plotMax.y - p.plotMin.y);

    return p;
}

float Projection::ToScreenX(double x) const {
    double spanX = bounds.maxX - bounds.minX;
    if (spanX <= 0.0) spanX = 1.0;
    return plotMin.x + static_cast<float>((x - bounds.minX) / spanX) * plotWidth;
}

float Projection::ToScreenY(double y) const {
    double spanY = bounds.maxY - bounds.minY;
    if (spanY <= 0.0) spanY = 1.0;
    // Ось Y направлена снизу вверх в графиках данных
    return plotMax.y - static_cast<float>((y - bounds.minY) / spanY) * plotHeight;
}

ImVec2 Projection::ToScreen(const ChartPoint& pt) const {
    return ImVec2(ToScreenX(pt.x), ToScreenY(pt.y));
}

double Projection::ToDataX(float screenX) const {
    if (plotWidth <= 0.0f) return bounds.minX;
    double spanX = bounds.maxX - bounds.minX;
    double frac = static_cast<double>(screenX - plotMin.x) / plotWidth;
    return bounds.minX + frac * spanX;
}

double Projection::ToDataY(float screenY) const {
    if (plotHeight <= 0.0f) return bounds.minY;
    double spanY = bounds.maxY - bounds.minY;
    double frac = static_cast<double>(plotMax.y - screenY) / plotHeight;
    return bounds.minY + frac * spanY;
}

ChartPoint Projection::ToData(const ImVec2& screenPt) const {
    return ChartPoint(ToDataX(screenPt.x), ToDataY(screenPt.y));
}

int NearestIndexByX(const std::vector<ChartPoint>& points, double targetX) {
    if (points.empty()) {
        return -1;
    }
    if (points.size() == 1) {
        return 0;
    }

    // Проверяем монотонность X для безопасного бинарного поиска
    bool isMonotonic = true;
    for (size_t i = 1; i < std::min<size_t>(points.size(), 16); ++i) {
        if (points[i].x < points[i - 1].x) {
            isMonotonic = false;
            break;
        }
    }

    if (isMonotonic) {
        int lo = 0;
        int hi = static_cast<int>(points.size()) - 1;
        while (hi - lo > 1) {
            int mid = (lo + hi) >> 1;
            if (points[static_cast<size_t>(mid)].x <= targetX) {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        double dLo = std::abs(points[static_cast<size_t>(lo)].x - targetX);
        double dHi = std::abs(points[static_cast<size_t>(hi)].x - targetX);
        return (dHi < dLo) ? hi : lo;
    }

    // Линейный поиск если массив не отсортирован
    int bestIdx = 0;
    double bestDist = std::abs(points[0].x - targetX);
    for (size_t i = 1; i < points.size(); ++i) {
        double dist = std::abs(points[i].x - targetX);
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = static_cast<int>(i);
        }
    }
    return bestIdx;
}

bool ClipInfiniteLineToRect(double x1, double y1, double x2, double y2,
                            double xmin, double ymin, double xmax, double ymax,
                            double& outX1, double& outY1, double& outX2, double& outY2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    if (std::hypot(dx, dy) < 1e-9) {
        return false;
    }

    double t0 = -1e9;
    double t1 = 1e9;

    double p[4] = { -dx, dx, -dy, dy };
    double q[4] = { x1 - xmin, xmax - x1, y1 - ymin, ymax - y1 };

    for (int i = 0; i < 4; ++i) {
        double pi = p[i];
        double qi = q[i];
        if (std::abs(pi) < 1e-12) {
            if (qi < 0.0) {
                return false;
            }
        } else if (pi < 0.0) {
            double t = qi / pi;
            if (t > t0) t0 = t;
        } else {
            double t = qi / pi;
            if (t < t1) t1 = t;
        }
    }

    if (t0 >= t1) {
        return false;
    }

    outX1 = x1 + t0 * dx;
    outY1 = y1 + t0 * dy;
    outX2 = x1 + t1 * dx;
    outY2 = y1 + t1 * dy;
    return true;
}

} // namespace ChartCore
