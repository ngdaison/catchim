#pragma once

#include "core/time/TimelineTime.h"
#include <vector>
#include <cmath>

namespace catchim::editor {

struct PathPoint {
    core::TimelineTime time;
    double x{0.0};
    double y{0.0};
};

struct EvaluatedPose {
    double x{0.0};
    double y{0.0};
    double headingDegrees{0.0};
    double distanceAlongPath{0.0};
};

class SpatialMotionPath {
public:
    SpatialMotionPath() = default;

    void addPoint(core::TimelineTime time, double x, double y);
    void clear();

    size_t pointCount() const noexcept { return points_.size(); }
    const std::vector<PathPoint>& points() const noexcept { return points_; }

    double totalLength() const;
    EvaluatedPose evaluateAt(core::TimelineTime time) const;

private:
    std::vector<PathPoint> points_;
};

} // namespace catchim::editor
