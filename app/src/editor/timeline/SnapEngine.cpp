#include "SnapEngine.h"
#include <cmath>
#include <limits>

namespace catchim::editor {

SnapResult SnapEngine::snap(
    const Timeline& timeline,
    core::TimelineTime targetTime,
    core::TimelineTime playheadTime,
    core::TimelineTime threshold,
    const std::optional<core::ClipId>& ignoreClipId
) {
    SnapResult result;
    result.snappedTime = targetTime;
    result.delta = core::TimelineTime(0);

    int64_t bestDist = threshold.ticks() + 1;
    SnapPoint bestPoint;
    bool found = false;

    // 1. Check Playhead
    int64_t distPlayhead = std::abs(targetTime.ticks() - playheadTime.ticks());
    if (distPlayhead <= threshold.ticks() && distPlayhead < bestDist) {
        bestDist = distPlayhead;
        bestPoint = {playheadTime, SnapPointType::Playhead, "playhead"};
        found = true;
    }

    // 2. Check Bookmarks
    for (const auto& bm : timeline.bookmarks()) {
        int64_t dist = std::abs(targetTime.ticks() - bm.time.ticks());
        if (dist <= threshold.ticks() && dist < bestDist) {
            bestDist = dist;
            bestPoint = {bm.time, SnapPointType::Bookmark, bm.id.str()};
            found = true;
        }
    }

    // 3. Check Clips
    for (const auto* track : timeline.allTracks()) {
        for (const auto& clip : track->clips()) {
            if (ignoreClipId.has_value() && clip.id() == *ignoreClipId) {
                continue;
            }

            // Clip start
            int64_t distStart = std::abs(targetTime.ticks() - clip.startTime().ticks());
            if (distStart <= threshold.ticks() && distStart < bestDist) {
                bestDist = distStart;
                bestPoint = {clip.startTime(), SnapPointType::ClipStart, clip.id().str()};
                found = true;
            }

            // Clip end
            int64_t distEnd = std::abs(targetTime.ticks() - clip.endTime().ticks());
            if (distEnd <= threshold.ticks() && distEnd < bestDist) {
                bestDist = distEnd;
                bestPoint = {clip.endTime(), SnapPointType::ClipEnd, clip.id().str()};
                found = true;
            }
        }
    }

    if (found) {
        result.snappedTime = bestPoint.time;
        result.delta = bestPoint.time - targetTime;
        result.hasSnapped = true;
        result.snapPoint = bestPoint;
    }

    return result;
}

} // namespace catchim::editor
