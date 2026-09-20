#include "editor/timeline/PlacementEngine.h"
#include "editor/timeline/Timeline.h"
#include <algorithm>

namespace catchim::editor {

namespace {

std::vector<int64_t> buildMaxEndThroughIndex(const std::vector<Clip>& clips) {
    std::vector<int64_t> maxEnds;
    maxEnds.reserve(clips.size());
    int64_t maxEnd = -1;
    for (const auto& clip : clips) {
        maxEnd = std::max(maxEnd, clip.endTime().ticks());
        maxEnds.push_back(maxEnd);
    }
    return maxEnds;
}

bool areClipsSortedByStartTime(const std::vector<Clip>& clips) {
    for (size_t i = 1; i < clips.size(); ++i) {
        if (clips[i - 1].startTime() > clips[i].startTime()) {
            return false;
        }
    }
    return true;
}

size_t findFirstClipStartingAtOrAfter(const std::vector<Clip>& clips, core::TimelineTime startTime) {
    size_t low = 0;
    size_t high = clips.size();
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (clips[mid].startTime() < startTime) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

bool wouldSortedClipOverlap(
    const std::vector<Clip>& clips,
    const std::vector<int64_t>& maxEndThroughIndex,
    core::TimelineTime startTime,
    core::TimelineTime endTime,
    const std::optional<core::ClipId>& excludeClipId
) {
    size_t candidateIndex = findFirstClipStartingAtOrAfter(clips, startTime);

    for (int64_t i = static_cast<int64_t>(candidateIndex) - 1; i >= 0; --i) {
        if (maxEndThroughIndex[static_cast<size_t>(i)] <= startTime.ticks()) {
            break;
        }
        const auto& clip = clips[static_cast<size_t>(i)];
        if (excludeClipId.has_value() && clip.id() == excludeClipId.value()) {
            continue;
        }
        if (clip.endTime() <= startTime) {
            continue;
        }
        return true;
    }

    for (size_t i = candidateIndex; i < clips.size(); ++i) {
        const auto& clip = clips[i];
        if (clip.startTime() >= endTime) {
            break;
        }
        if (excludeClipId.has_value() && clip.id() == excludeClipId.value()) {
            continue;
        }
        return true;
    }

    return false;
}

bool wouldClipOverlapLinear(
    const std::vector<Clip>& clips,
    core::TimelineTime startTime,
    core::TimelineTime endTime,
    const std::optional<core::ClipId>& excludeClipId
) {
    for (const auto& clip : clips) {
        if (excludeClipId.has_value() && clip.id() == excludeClipId.value()) {
            continue;
        }
        if (startTime < clip.endTime() && endTime > clip.startTime()) {
            return true;
        }
    }
    return false;
}

} // anonymous namespace

bool PlacementEngine::canPlaceTimeSpansOnTrack(
    const Track& track,
    const std::vector<PlacementTimeSpan>& timeSpans
) noexcept {
    const auto& clips = track.clips();
    if (clips.empty()) return true;

    bool isSorted = areClipsSortedByStartTime(clips);
    std::vector<int64_t> maxEnds;
    if (isSorted) {
        maxEnds = buildMaxEndThroughIndex(clips);
    }

    for (const auto& span : timeSpans) {
        core::TimelineTime endTime = span.startTime + span.duration;
        bool overlap = isSorted
            ? wouldSortedClipOverlap(clips, maxEnds, span.startTime, endTime, span.excludeClipId)
            : wouldClipOverlapLinear(clips, span.startTime, endTime, span.excludeClipId);

        if (overlap) {
            return false;
        }
    }
    return true;
}

bool PlacementEngine::canPlaceClipOnTrack(
    const Track& track,
    core::TimelineTime startTime,
    core::TimelineTime duration,
    const std::optional<core::ClipId>& excludeClipId
) noexcept {
    PlacementTimeSpan span{startTime, duration, excludeClipId};
    return canPlaceTimeSpansOnTrack(track, {span});
}

core::TimelineTime PlacementEngine::findAvailableGap(
    const Track& track,
    core::TimelineTime minStartTime,
    core::TimelineTime duration
) noexcept {
    if (track.clips().empty()) {
        return minStartTime;
    }

    // Collect non-excluded clips and sort
    std::vector<Clip> sortedClips = track.clips();
    std::sort(sortedClips.begin(), sortedClips.end(), [](const Clip& a, const Clip& b) {
        return a.startTime() < b.startTime();
    });

    core::TimelineTime candidate = minStartTime;

    for (const auto& clip : sortedClips) {
        if (clip.endTime() <= candidate) {
            continue;
        }
        if (candidate + duration <= clip.startTime()) {
            // Fits before this clip!
            return candidate;
        }
        // Advance candidate past this clip
        if (clip.endTime() > candidate) {
            candidate = clip.endTime();
        }
    }

    return candidate;
}

core::TimelineTime PlacementEngine::enforceMainTrackStart(
    const Track& mainTrack,
    core::TimelineTime requestedStartTime,
    const std::optional<core::ClipId>& excludeClipId
) noexcept {
    const Clip* earliest = nullptr;
    for (const auto& clip : mainTrack.clips()) {
        if (excludeClipId.has_value() && clip.id() == excludeClipId.value()) {
            continue;
        }
        if (!earliest || clip.startTime() < earliest->startTime()) {
            earliest = &clip;
        }
    }

    if (!earliest) {
        return core::TimelineTime(0);
    }

    if (requestedStartTime <= earliest->startTime()) {
        return core::TimelineTime(0);
    }

    return requestedStartTime;
}

TrackType PlacementEngine::getTrackTypeForClipType(ClipType clipType) noexcept {
    switch (clipType) {
        case ClipType::Video:
        case ClipType::Image:
            return TrackType::Video;
        case ClipType::Audio:
            return TrackType::Audio;
        case ClipType::Text:
            return TrackType::Text;
        case ClipType::Sticker:
        case ClipType::Graphic:
            return TrackType::Graphic;
        case ClipType::Effect:
            return TrackType::Effect;
    }
    return TrackType::Video;
}

bool PlacementEngine::canClipGoOnTrack(ClipType clipType, TrackType trackType) noexcept {
    return getTrackTypeForClipType(clipType) == trackType;
}

size_t PlacementEngine::getDefaultInsertIndexForTrack(
    const Timeline& timeline,
    TrackType trackType
) noexcept {
    size_t overlayCount = timeline.overlayTracks().size();
    size_t audioCount = timeline.audioTracks().size();

    if (trackType == TrackType::Audio) {
        return overlayCount + 1 + audioCount;
    }
    if (trackType == TrackType::Effect) {
        return 0;
    }
    return overlayCount;
}

TrackPlacementResult PlacementEngine::resolvePreferredTrackPlacement(
    const Timeline& timeline,
    TrackType trackType,
    size_t preferredIndex,
    const std::string& direction
) noexcept {
    size_t overlayCount = timeline.overlayTracks().size();
    size_t totalTracks = overlayCount + 1 + timeline.audioTracks().size();

    TrackPlacementResult result;
    result.kind = TrackPlacementResult::Kind::NewTrack;
    result.trackType = trackType;

    if (totalTracks == 0) {
        result.insertIndex = 0;
        result.insertPosition = (trackType == TrackType::Audio) ? "below" : "above";
        return result;
    }

    size_t safePreferred = std::min(preferredIndex, totalTracks - 1);
    size_t mainIndex = overlayCount;

    if (trackType == TrackType::Audio) {
        if (safePreferred <= mainIndex) {
            result.insertIndex = mainIndex + 1;
            result.insertPosition = "below";
            return result;
        }
        result.insertIndex = (direction == "above") ? safePreferred : safePreferred + 1;
        result.insertPosition = direction;
        return result;
    }

    size_t insertIdx = (direction == "above") ? safePreferred : safePreferred + 1;
    if (insertIdx > mainIndex) {
        result.insertIndex = mainIndex;
        result.insertPosition = "above";
        return result;
    }

    result.insertIndex = insertIdx;
    result.insertPosition = direction;
    return result;
}

} // namespace catchim::editor
