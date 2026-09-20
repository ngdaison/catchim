#include "editor/animation/SpatialMotionPath.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::editor {

void SpatialMotionPath::addPoint(core::TimelineTime time, double x, double y) {
    auto it = std::lower_bound(points_.begin(), points_.end(), time, [](const PathPoint& pt, core::TimelineTime t) {
        return pt.time < t;
    });
    points_.insert(it, PathPoint{time, x, y});
}

void SpatialMotionPath::clear() {
    points_.clear();
}

double SpatialMotionPath::totalLength() const {
    if (points_.size() < 2) return 0.0;
    double dist = 0.0;
    for (size_t i = 0; i + 1 < points_.size(); ++i) {
        double dx = points_[i + 1].x - points_[i].x;
        double dy = points_[i + 1].y - points_[i].y;
        dist += std::sqrt(dx * dx + dy * dy);
    }
    return dist;
}

EvaluatedPose SpatialMotionPath::evaluateAt(core::TimelineTime time) const {
    if (points_.empty()) {
        return {0.0, 0.0, 0.0, 0.0};
    }
    if (points_.size() == 1) {
        return {points_[0].x, points_[0].y, 0.0, 0.0};
    }

    if (time <= points_.front().time) {
        double dx = points_[1].x - points_[0].x;
        double dy = points_[1].y - points_[0].y;
        double heading = std::atan2(dy, dx) * 180.0 / M_PI;
        return {points_.front().x, points_.front().y, heading, 0.0};
    }

    if (time >= points_.back().time) {
        size_t n = points_.size();
        double dx = points_[n - 1].x - points_[n - 2].x;
        double dy = points_[n - 1].y - points_[n - 2].y;
        double heading = std::atan2(dy, dx) * 180.0 / M_PI;
        return {points_.back().x, points_.back().y, heading, totalLength()};
    }

    // Find segment [i, i+1]
    size_t i = 0;
    double accumulatedDist = 0.0;
    for (size_t k = 0; k + 1 < points_.size(); ++k) {
        if (time >= points_[k].time && time <= points_[k + 1].time) {
            i = k;
            break;
        }
        double dx = points_[k + 1].x - points_[k].x;
        double dy = points_[k + 1].y - points_[k].y;
        accumulatedDist += std::sqrt(dx * dx + dy * dy);
    }

    double dt = (points_[i + 1].time - points_[i].time).toSeconds();
    double u = dt > 0.0 ? (time - points_[i].time).toSeconds() / dt : 0.0;
    u = std::clamp(u, 0.0, 1.0);

    // Catmull-Rom control points
    PathPoint p0 = (i > 0) ? points_[i - 1] : points_[i];
    PathPoint p1 = points_[i];
    PathPoint p2 = points_[i + 1];
    PathPoint p3 = (i + 2 < points_.size()) ? points_[i + 2] : points_[i + 1];

    // Spline evaluation
    double u2 = u * u;
    double u3 = u2 * u;

    auto catmullRom = [u, u2, u3](double a, double b, double c, double d) {
        return 0.5 * ((2.0 * b) +
                      (-a + c) * u +
                      (2.0 * a - 5.0 * b + 4.0 * c - d) * u2 +
                      (-a + 3.0 * b - 3.0 * c + d) * u3);
    };

    auto catmullRomDeriv = [u, u2](double a, double b, double c, double d) {
        return 0.5 * ((-a + c) +
                      2.0 * (2.0 * a - 5.0 * b + 4.0 * c - d) * u +
                      3.0 * (-a + 3.0 * b - 3.0 * c + d) * u2);
    };

    double posX = catmullRom(p0.x, p1.x, p2.x, p3.x);
    double posY = catmullRom(p0.y, p1.y, p2.y, p3.y);

    double dx = catmullRomDeriv(p0.x, p1.x, p2.x, p3.x);
    double dy = catmullRomDeriv(p0.y, p1.y, p2.y, p3.y);

    double heading = std::atan2(dy, dx) * 180.0 / M_PI;

    double segDx = points_[i + 1].x - points_[i].x;
    double segDy = points_[i + 1].y - points_[i].y;
    double dist = accumulatedDist + u * std::sqrt(segDx * segDx + segDy * segDy);

    return {posX, posY, heading, dist};
}

} // namespace catchim::editor
