#pragma once

#include "core/time/TimelineTime.h"
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <cstdint>

namespace catchim::editor::snapping {

enum class TimelineSnapPointType {
    ElementStart,
    ElementEnd,
    Playhead,
    Bookmark,
    Keyframe
};
using SnapPointType = TimelineSnapPointType;

inline const char* snapPointTypeToString(TimelineSnapPointType type) noexcept {
    switch (type) {
        case TimelineSnapPointType::ElementStart: return "element-start";
        case TimelineSnapPointType::ElementEnd: return "element-end";
        case TimelineSnapPointType::Playhead: return "playhead";
        case TimelineSnapPointType::Bookmark: return "bookmark";
        case TimelineSnapPointType::Keyframe: return "keyframe";
    }
    return "element-start";
}

struct TimelineSnapPoint {
    core::TimelineTime time{0};
    TimelineSnapPointType type{TimelineSnapPointType::ElementStart};
    std::string elementId;
    std::string trackId;

    bool operator==(const TimelineSnapPoint& other) const = default;
};
using SnapPoint = TimelineSnapPoint;

struct TimelineSnapResult {
    core::TimelineTime snappedTime{0};
    std::optional<TimelineSnapPoint> snapPoint{std::nullopt};
    int64_t snapDistanceTicks{0};
};
using SnapResult = TimelineSnapResult;

using TimelineSnapPointSource = std::function<std::vector<TimelineSnapPoint>()>;

/**
 * @brief Core timeline snapping engine with linear and binary-search resolution.
 * Corresponds to web/src/timeline/snapping/ (types.ts, build.ts, resolve.ts, threshold.ts).
 */
class TimelineSnappingEngine {
public:
    static constexpr double DEFAULT_TIMELINE_SNAP_THRESHOLD_PX = 10.0;
    static constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 100.0;

    static core::TimelineTime getTimelineSnapThresholdInTicks(
        double zoomLevel,
        double snapThresholdPx = DEFAULT_TIMELINE_SNAP_THRESHOLD_PX
    ) noexcept;

    static std::vector<SnapPoint> buildTimelineSnapPoints(
        const std::vector<TimelineSnapPointSource>& sources
    );

    static std::vector<SnapPoint> buildSortedTimelineSnapPoints(
        const std::vector<TimelineSnapPointSource>& sources
    );

    static SnapResult resolveTimelineSnap(
        core::TimelineTime targetTime,
        const std::vector<SnapPoint>& snapPoints,
        core::TimelineTime maxSnapDistance
    ) noexcept;

    static SnapResult resolveSortedTimelineSnap(
        core::TimelineTime targetTime,
        const std::vector<SnapPoint>& sortedSnapPoints,
        core::TimelineTime maxSnapDistance
    ) noexcept;

    static SnapResult resolveTimelineSnapLinear(
        core::TimelineTime targetTime,
        const std::vector<SnapPoint>& snapPoints,
        core::TimelineTime maxSnapDistance
    ) noexcept;

    static bool isSortedByTime(const std::vector<SnapPoint>& snapPoints) noexcept;
};

} // namespace catchim::editor::snapping
