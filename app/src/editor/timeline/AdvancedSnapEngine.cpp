#include "editor/timeline/AdvancedSnapEngine.h"
#include <cmath>
#include <algorithm>

namespace catchim::editor {

core::TimelineTime AdvancedSnapEngine::getTimelineSnapThresholdInTicks(
    double zoomLevel,
    double snapThresholdPx
) noexcept {
    if (zoomLevel <= 0.001) zoomLevel = 0.001;
    double pixelsPerSecond = BASE_PIXELS_PER_SECOND * zoomLevel;
    double seconds = snapThresholdPx / pixelsPerSecond;
    return core::TimelineTime::fromSeconds(seconds);
}

AdvancedSnapResult AdvancedSnapEngine::resolveSortedTimelineSnap(
    core::TimelineTime targetTime,
    const std::vector<AdvancedSnapPoint>& sortedSnapPoints,
    core::TimelineTime maxSnapDistance
) noexcept {
    AdvancedSnapResult result;
    result.snappedTime = targetTime;
    result.hasSnapped = false;

    if (sortedSnapPoints.empty() || maxSnapDistance.ticks() <= 0) {
        return result;
    }

    core::TimelineTime searchStart = (targetTime.ticks() >= maxSnapDistance.ticks())
        ? (targetTime - maxSnapDistance)
        : core::TimelineTime(0);

    auto it = std::lower_bound(
        sortedSnapPoints.begin(),
        sortedSnapPoints.end(),
        searchStart,
        [](const AdvancedSnapPoint& pt, core::TimelineTime t) {
            return pt.time < t;
        }
    );

    int64_t closestDistance = std::numeric_limits<int64_t>::max();
    const AdvancedSnapPoint* bestPoint = nullptr;

    core::TimelineTime searchEnd = targetTime + maxSnapDistance;

    for (; it != sortedSnapPoints.end() && it->time <= searchEnd; ++it) {
        int64_t dist = std::abs(targetTime.ticks() - it->time.ticks());
        if (dist <= maxSnapDistance.ticks() && dist < closestDistance) {
            closestDistance = dist;
            bestPoint = &(*it);
        }
    }

    if (bestPoint) {
        result.snappedTime = bestPoint->time;
        result.snapPoint = *bestPoint;
        result.snapDistance = core::TimelineTime::fromTicks(closestDistance);
        result.hasSnapped = true;
    }

    return result;
}

std::vector<AdvancedSnapPoint> AdvancedSnapEngine::collectAllSnapPoints(
    const Timeline& timeline,
    core::TimelineTime playheadTime,
    bool includeKeyframes,
    const std::optional<core::ClipId>& ignoreClipId
) {
    std::vector<AdvancedSnapPoint> points;

    // 1. Playhead
    points.push_back(AdvancedSnapPoint{
        playheadTime,
        AdvancedSnapType::Playhead,
        "playhead",
        std::nullopt
    });

    // 2. Bookmarks
    for (const auto& bm : timeline.bookmarks()) {
        points.push_back(AdvancedSnapPoint{
            bm.time,
            AdvancedSnapType::Bookmark,
            bm.id.str(),
            std::nullopt
        });
    }

    // 3. Clips & Keyframes across all tracks
    for (const auto* track : timeline.allTracks()) {
        if (!track) continue;

        for (const auto& clip : track->clips()) {
            if (ignoreClipId.has_value() && clip.id() == ignoreClipId.value()) {
                continue;
            }

            // Clip boundaries
            points.push_back(AdvancedSnapPoint{
                clip.startTime(),
                AdvancedSnapType::ClipStart,
                clip.id().str(),
                track->id()
            });

            points.push_back(AdvancedSnapPoint{
                clip.endTime(),
                AdvancedSnapType::ClipEnd,
                clip.id().str(),
                track->id()
            });

            // Keyframes
            if (includeKeyframes && clip.params().contains("animations") && clip.params()["animations"].is_object()) {
                const auto& anims = clip.params()["animations"];
                for (auto it = anims.begin(); it != anims.end(); ++it) {
                    if (it.value().is_object() && it.value().contains("keys") && it.value()["keys"].is_array()) {
                        for (const auto& keyItem : it.value()["keys"]) {
                            if (keyItem.contains("time")) {
                                int64_t kfTicks = 0;
                                if (keyItem["time"].is_number()) {
                                    kfTicks = keyItem["time"].get<int64_t>();
                                }
                                core::TimelineTime kfTime = clip.startTime() + core::TimelineTime::fromTicks(kfTicks);
                                points.push_back(AdvancedSnapPoint{
                                    kfTime,
                                    AdvancedSnapType::Keyframe,
                                    clip.id().str(),
                                    track->id()
                                });
                            }
                        }
                    }
                }
            }
        }
    }

    // Sort all points by timestamp
    std::sort(points.begin(), points.end(), [](const AdvancedSnapPoint& a, const AdvancedSnapPoint& b) {
        return a.time < b.time;
    });

    return points;
}

} // namespace catchim::editor
