#pragma once

#include "editor/timeline/Timeline.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <optional>
#include <string>

namespace catchim::editor {

enum class AdvancedSnapType {
    Playhead,
    ClipStart,
    ClipEnd,
    Bookmark,
    Keyframe
};

struct AdvancedSnapPoint {
    core::TimelineTime time{0};
    AdvancedSnapType type{AdvancedSnapType::ClipStart};
    std::string targetId;
    std::optional<core::TrackId> trackId{std::nullopt};
};

struct AdvancedSnapResult {
    core::TimelineTime snappedTime{0};
    std::optional<AdvancedSnapPoint> snapPoint{std::nullopt};
    core::TimelineTime snapDistance{0};
    bool hasSnapped{false};
};

class AdvancedSnapEngine {
public:
    static constexpr double BASE_PIXELS_PER_SECOND = 100.0;
    static constexpr double DEFAULT_SNAP_THRESHOLD_PX = 10.0;

    // Calculates snap threshold in timeline ticks adapted to the current zoom level
    static core::TimelineTime getTimelineSnapThresholdInTicks(
        double zoomLevel,
        double snapThresholdPx = DEFAULT_SNAP_THRESHOLD_PX
    ) noexcept;

    // Fast binary search snapping against sorted snap points
    static AdvancedSnapResult resolveSortedTimelineSnap(
        core::TimelineTime targetTime,
        const std::vector<AdvancedSnapPoint>& sortedSnapPoints,
        core::TimelineTime maxSnapDistance
    ) noexcept;

    // Collects and sorts all snap points from clips, bookmarks, playhead and keyframes
    static std::vector<AdvancedSnapPoint> collectAllSnapPoints(
        const Timeline& timeline,
        core::TimelineTime playheadTime,
        bool includeKeyframes = true,
        const std::optional<core::ClipId>& ignoreClipId = std::nullopt
    );
};

} // namespace catchim::editor
