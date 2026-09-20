#pragma once

#include "Timeline.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <optional>

namespace catchim::editor {

enum class SnapPointType {
    Playhead,
    ClipStart,
    ClipEnd,
    Bookmark
};

struct SnapPoint {
    core::TimelineTime time;
    SnapPointType type;
    std::string targetId; // ClipId, BookmarkId, etc.
};

struct SnapResult {
    core::TimelineTime snappedTime;
    core::TimelineTime delta;
    bool hasSnapped{false};
    std::optional<SnapPoint> snapPoint{std::nullopt};
};

class SnapEngine {
public:
    static SnapResult snap(
        const Timeline& timeline,
        core::TimelineTime targetTime,
        core::TimelineTime playheadTime,
        core::TimelineTime threshold,
        const std::optional<core::ClipId>& ignoreClipId = std::nullopt
    );
};

} // namespace catchim::editor
