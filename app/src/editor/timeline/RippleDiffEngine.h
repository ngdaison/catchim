#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include "editor/timeline/Timeline.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <map>

namespace catchim::editor {

struct TimeInterval {
    core::TimelineTime startTime{0};
    core::TimelineTime endTime{0};

    bool operator==(const TimeInterval& other) const noexcept {
        return startTime == other.startTime && endTime == other.endTime;
    }
};

struct RippleAdjustment {
    core::TrackId trackId{core::TrackId::empty()};
    core::TimelineTime afterTime{0};
    core::TimelineTime shiftAmount{0};

    bool operator==(const RippleAdjustment& other) const noexcept {
        return trackId == other.trackId && afterTime == other.afterTime && shiftAmount == other.shiftAmount;
    }
};

class RippleDiffEngine {
public:
    static std::vector<TimeInterval> normalizeIntervals(
        const std::vector<TimeInterval>& intervals
    );

    static std::vector<TimeInterval> subtractSingleInterval(
        const TimeInterval& sourceInterval,
        const std::vector<TimeInterval>& overlappingIntervals
    );

    static std::vector<TimeInterval> subtractIntervalSets(
        const std::vector<TimeInterval>& sourceIntervals,
        const std::vector<TimeInterval>& overlappingIntervals
    );

    static std::vector<RippleAdjustment> computeTrackRippleAdjustments(
        const core::TrackId& trackId,
        const std::vector<Clip>& beforeElements,
        const std::vector<Clip>& afterElements,
        const std::unordered_set<std::string>& allAfterElementIds
    );

    static std::vector<RippleAdjustment> computeRippleAdjustments(
        const Timeline& beforeTimeline,
        const Timeline& afterTimeline
    );

    static void applyRippleAdjustments(
        Timeline& timeline,
        const std::vector<RippleAdjustment>& adjustments
    );
};

} // namespace catchim::editor
