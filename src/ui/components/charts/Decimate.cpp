#include "ui/components/charts/Decimate.hpp"
#include <cmath>
#include <algorithm>

namespace ChartCore {

std::vector<ChartPoint> DecimateMinMax(const std::vector<ChartPoint>& points, int columns) {
    if (points.size() <= static_cast<size_t>(columns * 2) || columns < 2) {
        return points;
    }

    double xMin = points[0].x;
    double xMax = points[0].x;
    for (const auto& p : points) {
        if (p.x < xMin) xMin = p.x;
        if (p.x > xMax) xMax = p.x;
    }

    double span = xMax - xMin;
    if (span <= 0.0) {
        return points;
    }

    double bucketSize = span / columns;

    std::vector<ChartPoint> out;
    out.reserve(static_cast<size_t>(columns * 2));

    int currentBucket = -1;
    bool hasMin = false;
    bool hasMax = false;
    ChartPoint minPt;
    ChartPoint maxPt;
    double minY = 0.0;
    double maxY = 0.0;
    size_t minSeq = 0;
    size_t maxSeq = 0;
    size_t seq = 0;

    auto flush = [&]() {
        if (!hasMin || !hasMax) return;
        if (minPt.x == maxPt.x && minPt.y == maxPt.y) {
            out.push_back(minPt);
        } else if (minSeq < maxSeq) {
            out.push_back(minPt);
            out.push_back(maxPt);
        } else {
            out.push_back(maxPt);
            out.push_back(minPt);
        }
        hasMin = false;
        hasMax = false;
    };

    for (const auto& p : points) {
        int b = static_cast<int>(std::floor((p.x - xMin) / bucketSize));
        if (b < 0) b = 0;
        if (b >= columns) b = columns - 1;

        if (b != currentBucket) {
            flush();
            currentBucket = b;
            minY = p.y;
            maxY = p.y;
            minPt = p;
            maxPt = p;
            minSeq = seq++;
            maxSeq = minSeq;
            hasMin = true;
            hasMax = true;
        } else {
            if (p.y < minY) {
                minY = p.y;
                minPt = p;
                minSeq = seq++;
            }
            if (p.y > maxY) {
                maxY = p.y;
                maxPt = p;
                maxSeq = seq++;
            }
        }
    }
    flush();

    return out;
}

} // namespace ChartCore
