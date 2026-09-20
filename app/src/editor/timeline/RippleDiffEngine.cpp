#include "editor/timeline/RippleDiffEngine.h"
#include <algorithm>

namespace catchim::editor {

namespace {

void pushInterval(std::vector<TimeInterval>& intervals, core::TimelineTime startTime, core::TimelineTime endTime) {
    if (endTime <= startTime) {
        return;
    }
    intervals.push_back(TimeInterval{.startTime = startTime, .endTime = endTime});
}

} // namespace

std::vector<TimeInterval> RippleDiffEngine::normalizeIntervals(
    const std::vector<TimeInterval>& intervals
) {
    std::vector<TimeInterval> validIntervals;
    validIntervals.reserve(intervals.size());
    for (const auto& interval : intervals) {
        pushInterval(validIntervals, interval.startTime, interval.endTime);
    }

    if (validIntervals.empty()) {
        return {};
    }

    std::sort(validIntervals.begin(), validIntervals.end(), [](const TimeInterval& a, const TimeInterval& b) {
        return a.startTime < b.startTime;
    });

    std::vector<TimeInterval> merged;
    merged.push_back(validIntervals[0]);

    for (size_t i = 1; i < validIntervals.size(); ++i) {
        auto& previous = merged.back();
        const auto& current = validIntervals[i];

        if (current.startTime <= previous.endTime) {
            previous.endTime = std::max(previous.endTime, current.endTime);
        } else {
            merged.push_back(current);
        }
    }

    return merged;
}

std::vector<TimeInterval> RippleDiffEngine::subtractSingleInterval(
    const TimeInterval& sourceInterval,
    const std::vector<TimeInterval>& overlappingIntervals
) {
    std::vector<TimeInterval> remainingIntervals = {sourceInterval};

    for (const auto& overlap : overlappingIntervals) {
        std::vector<TimeInterval> nextRemaining;
        for (const auto& rem : remainingIntervals) {
            if (overlap.endTime <= rem.startTime || overlap.startTime >= rem.endTime) {
                nextRemaining.push_back(rem);
                continue;
            }

            pushInterval(nextRemaining, rem.startTime, overlap.startTime);
            pushInterval(nextRemaining, overlap.endTime, rem.endTime);
        }

        remainingIntervals = std::move(nextRemaining);
        if (remainingIntervals.empty()) {
            return {};
        }
    }

    return remainingIntervals;
}

std::vector<TimeInterval> RippleDiffEngine::subtractIntervalSets(
    const std::vector<TimeInterval>& sourceIntervals,
    const std::vector<TimeInterval>& overlappingIntervals
) {
    const auto normalizedSource = normalizeIntervals(sourceIntervals);
    const auto normalizedOverlap = normalizeIntervals(overlappingIntervals);

    std::vector<TimeInterval> result;
    for (const auto& source : normalizedSource) {
        const auto sub = subtractSingleInterval(source, normalizedOverlap);
        result.insert(result.end(), sub.begin(), sub.end());
    }

    return result;
}

std::vector<RippleAdjustment> RippleDiffEngine::computeTrackRippleAdjustments(
    const core::TrackId& trackId,
    const std::vector<Clip>& beforeElements,
    const std::vector<Clip>& afterElements,
    const std::unordered_set<std::string>& allAfterElementIds
) {
    std::map<std::string, TimeInterval> beforeMap;
    for (const auto& clip : beforeElements) {
        beforeMap[clip.id().str()] = TimeInterval{
            .startTime = clip.startTime(),
            .endTime = clip.endTime()
        };
    }

    std::map<std::string, TimeInterval> afterMap;
    for (const auto& clip : afterElements) {
        afterMap[clip.id().str()] = TimeInterval{
            .startTime = clip.startTime(),
            .endTime = clip.endTime()
        };
    }

    std::vector<TimeInterval> vacatedIntervals;
    std::vector<TimeInterval> joinedIntervals;

    for (const auto& [id, beforeSpan] : beforeMap) {
        const auto it = afterMap.find(id);
        if (it == afterMap.end()) {
            const bool wasMovedToAnotherTrack = (allAfterElementIds.find(id) != allAfterElementIds.end());
            if (!wasMovedToAnotherTrack) {
                pushInterval(vacatedIntervals, beforeSpan.startTime, beforeSpan.endTime);
            }
            continue;
        }

        const auto& afterSpan = it->second;
        if (beforeSpan.endTime > afterSpan.endTime) {
            pushInterval(vacatedIntervals, afterSpan.endTime, beforeSpan.endTime);
        }
    }

    for (const auto& [id, afterSpan] : afterMap) {
        if (beforeMap.find(id) == beforeMap.end()) {
            pushInterval(joinedIntervals, afterSpan.startTime, afterSpan.endTime);
        }
    }

    const auto freedIntervals = subtractIntervalSets(vacatedIntervals, joinedIntervals);

    std::vector<RippleAdjustment> adjustments;
    for (const auto& interval : freedIntervals) {
        const auto shiftAmount = interval.endTime - interval.startTime;
        if (shiftAmount > core::TimelineTime::zero()) {
            adjustments.push_back(RippleAdjustment{
                .trackId = trackId,
                .afterTime = interval.endTime,
                .shiftAmount = shiftAmount
            });
        }
    }

    return adjustments;
}

std::vector<RippleAdjustment> RippleDiffEngine::computeRippleAdjustments(
    const Timeline& beforeTimeline,
    const Timeline& afterTimeline
) {
    const auto beforeTracks = beforeTimeline.allTracks();
    const auto afterTracks = afterTimeline.allTracks();

    std::unordered_set<std::string> allAfterElementIds;
    std::map<core::TrackId, const Track*> afterTracksById;
    for (const auto* track : afterTracks) {
        afterTracksById[track->id()] = track;
        for (const auto& clip : track->clips()) {
            allAfterElementIds.insert(clip.id().str());
        }
    }

    std::vector<RippleAdjustment> allAdjustments;

    for (const auto* beforeTrack : beforeTracks) {
        const auto it = afterTracksById.find(beforeTrack->id());
        const std::vector<Clip>& afterClips = (it != afterTracksById.end()) ? it->second->clips() : std::vector<Clip>{};

        const auto trackAdjustments = computeTrackRippleAdjustments(
            beforeTrack->id(),
            beforeTrack->clips(),
            afterClips,
            allAfterElementIds
        );
        allAdjustments.insert(allAdjustments.end(), trackAdjustments.begin(), trackAdjustments.end());
    }

    return allAdjustments;
}

void RippleDiffEngine::applyRippleAdjustments(
    Timeline& timeline,
    const std::vector<RippleAdjustment>& adjustments
) {
    if (adjustments.empty()) {
        return;
    }

    // Group adjustments by trackId
    std::map<core::TrackId, std::vector<RippleAdjustment>> byTrack;
    for (const auto& adj : adjustments) {
        byTrack[adj.trackId].push_back(adj);
    }

    for (auto& [trackId, trackAdjustments] : byTrack) {
        Track* track = timeline.findTrack(trackId);
        if (!track) continue;

        // Sort descending by afterTime
        std::sort(trackAdjustments.begin(), trackAdjustments.end(), [](const RippleAdjustment& a, const RippleAdjustment& b) {
            return a.afterTime > b.afterTime;
        });

        for (const auto& adj : trackAdjustments) {
            for (auto& clip : track->clips()) {
                if (clip.startTime() >= adj.afterTime) {
                    clip.setStartTime(clip.startTime() - adj.shiftAmount);
                }
            }
        }
    }
}

} // namespace catchim::editor
