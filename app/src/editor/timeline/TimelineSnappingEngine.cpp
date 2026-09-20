#include "editor/timeline/TimelineSnappingEngine.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace catchim::editor::snapping {

core::TimelineTime TimelineSnappingEngine::getTimelineSnapThresholdInTicks(
    double zoomLevel,
    double snapThresholdPx
) noexcept {
    if (zoomLevel <= 0.0) {
        return core::TimelineTime(0);
    }
    double pixelsPerSecond = BASE_TIMELINE_PIXELS_PER_SECOND * zoomLevel;
    double seconds = snapThresholdPx / pixelsPerSecond;
    return core::TimelineTime::fromSeconds(seconds);
}

std::vector<SnapPoint> TimelineSnappingEngine::buildTimelineSnapPoints(
    const std::vector<TimelineSnapPointSource>& sources
) {
    std::vector<SnapPoint> snapPoints;
    for (const auto& source : sources) {
        if (source) {
            auto pts = source();
            snapPoints.insert(snapPoints.end(), pts.begin(), pts.end());
        }
    }
    return snapPoints;
}

std::vector<SnapPoint> TimelineSnappingEngine::buildSortedTimelineSnapPoints(
    const std::vector<TimelineSnapPointSource>& sources
) {
    auto pts = buildTimelineSnapPoints(sources);
    std::sort(pts.begin(), pts.end(), [](const SnapPoint& a, const SnapPoint& b) {
        return a.time < b.time;
    });
    return pts;
}

bool TimelineSnappingEngine::isSortedByTime(const std::vector<SnapPoint>& snapPoints) noexcept {
    for (size_t i = 1; i < snapPoints.size(); ++i) {
        if (snapPoints[i - 1].time > snapPoints[i].time) {
            return false;
        }
    }
    return true;
}

SnapResult TimelineSnappingEngine::resolveTimelineSnapLinear(
    core::TimelineTime targetTime,
    const std::vector<SnapPoint>& snapPoints,
    core::TimelineTime maxSnapDistance
) noexcept {
    std::optional<SnapPoint> closestSnapPoint = std::nullopt;
    int64_t closestDistance = std::numeric_limits<int64_t>::max();
    int64_t maxDistTicks = std::max<int64_t>(0, maxSnapDistance.ticks());

    for (const auto& pt : snapPoints) {
        int64_t distance = std::abs(targetTime.ticks() - pt.time.ticks());
        if (distance <= maxDistTicks && distance < closestDistance) {
            closestDistance = distance;
            closestSnapPoint = pt;
        }
    }

    if (closestSnapPoint.has_value()) {
        return SnapResult{
            .snappedTime = closestSnapPoint->time,
            .snapPoint = closestSnapPoint,
            .snapDistanceTicks = closestDistance
        };
    }

    return SnapResult{
        .snappedTime = targetTime,
        .snapPoint = std::nullopt,
        .snapDistanceTicks = closestDistance
    };
}

SnapResult TimelineSnappingEngine::resolveSortedTimelineSnap(
    core::TimelineTime targetTime,
    const std::vector<SnapPoint>& sortedSnapPoints,
    core::TimelineTime maxSnapDistance
) noexcept {
    if (sortedSnapPoints.empty()) {
        return SnapResult{
            .snappedTime = targetTime,
            .snapPoint = std::nullopt,
            .snapDistanceTicks = std::numeric_limits<int64_t>::max()
        };
    }

    int64_t maxDistTicks = std::max<int64_t>(0, maxSnapDistance.ticks());
    core::TimelineTime searchStartTime(std::max<int64_t>(0, targetTime.ticks() - maxDistTicks));
    core::TimelineTime searchEndTime = targetTime + maxSnapDistance;

    auto it = std::lower_bound(
        sortedSnapPoints.begin(),
        sortedSnapPoints.end(),
        searchStartTime,
        [](const SnapPoint& pt, core::TimelineTime t) {
            return pt.time < t;
        }
    );

    std::optional<SnapPoint> closestSnapPoint = std::nullopt;
    int64_t closestDistance = std::numeric_limits<int64_t>::max();

    for (; it != sortedSnapPoints.end() && it->time <= searchEndTime; ++it) {
        int64_t distance = std::abs(targetTime.ticks() - it->time.ticks());
        if (distance <= maxDistTicks && distance < closestDistance) {
            closestDistance = distance;
            closestSnapPoint = *it;
        }
    }

    if (closestSnapPoint.has_value()) {
        return SnapResult{
            .snappedTime = closestSnapPoint->time,
            .snapPoint = closestSnapPoint,
            .snapDistanceTicks = closestDistance
        };
    }

    return SnapResult{
        .snappedTime = targetTime,
        .snapPoint = std::nullopt,
        .snapDistanceTicks = closestDistance
    };
}

SnapResult TimelineSnappingEngine::resolveTimelineSnap(
    core::TimelineTime targetTime,
    const std::vector<SnapPoint>& snapPoints,
    core::TimelineTime maxSnapDistance
) noexcept {
    if (isSortedByTime(snapPoints)) {
        return resolveSortedTimelineSnap(targetTime, snapPoints, maxSnapDistance);
    }
    return resolveTimelineSnapLinear(targetTime, snapPoints, maxSnapDistance);
}

} // namespace catchim::editor::snapping
