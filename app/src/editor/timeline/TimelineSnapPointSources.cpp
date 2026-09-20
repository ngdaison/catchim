#include "editor/timeline/TimelineSnapPointSources.h"
#include "editor/timeline/Timeline.h"

namespace catchim::editor::snapping {

std::vector<SnapPoint> TimelineSnapPointSources::getElementEdgeSnapPoints(
    const std::vector<Track>& tracks,
    const std::unordered_set<std::string>& excludeElementIds
) {
    std::vector<SnapPoint> snapPoints;

    for (const auto& track : tracks) {
        for (const auto& clip : track.clips()) {
            if (excludeElementIds.contains(clip.id().str())) {
                continue;
            }

            snapPoints.push_back(SnapPoint{
                .time = clip.startTime(),
                .type = SnapPointType::ElementStart,
                .elementId = clip.id().str(),
                .trackId = track.id().str()
            });

            snapPoints.push_back(SnapPoint{
                .time = clip.endTime(),
                .type = SnapPointType::ElementEnd,
                .elementId = clip.id().str(),
                .trackId = track.id().str()
            });
        }
    }

    return snapPoints;
}

std::vector<SnapPoint> TimelineSnapPointSources::getElementEdgeSnapPoints(
    const Timeline& timeline,
    const std::unordered_set<std::string>& excludeElementIds
) {
    std::vector<SnapPoint> snapPoints;

    for (const auto* track : timeline.allTracks()) {
        if (!track) continue;
        for (const auto& clip : track->clips()) {
            if (excludeElementIds.contains(clip.id().str())) {
                continue;
            }

            snapPoints.push_back(SnapPoint{
                .time = clip.startTime(),
                .type = SnapPointType::ElementStart,
                .elementId = clip.id().str(),
                .trackId = track->id().str()
            });

            snapPoints.push_back(SnapPoint{
                .time = clip.endTime(),
                .type = SnapPointType::ElementEnd,
                .elementId = clip.id().str(),
                .trackId = track->id().str()
            });
        }
    }

    return snapPoints;
}

std::vector<SnapPoint> TimelineSnapPointSources::getPlayheadSnapPoints(
    core::TimelineTime playheadTime
) {
    return {
        SnapPoint{
            .time = playheadTime,
            .type = SnapPointType::Playhead,
            .elementId = "",
            .trackId = ""
        }
    };
}

std::vector<SnapPoint> TimelineSnapPointSources::getAnimationKeyframeSnapPoints(
    const std::vector<Track>& tracks,
    const std::unordered_set<std::string>& excludeElementIds
) {
    std::vector<SnapPoint> snapPoints;

    for (const auto& track : tracks) {
        for (const auto& clip : track.clips()) {
            if (excludeElementIds.contains(clip.id().str())) {
                continue;
            }

            for (const auto& [prop, channel] : clip.animationChannels()) {
                for (const auto& kf : channel.keyframes()) {
                    snapPoints.push_back(SnapPoint{
                        .time = clip.startTime() + kf.time,
                        .type = SnapPointType::Keyframe,
                        .elementId = clip.id().str(),
                        .trackId = track.id().str()
                    });
                }
            }
        }
    }

    return snapPoints;
}

std::vector<SnapPoint> TimelineSnapPointSources::getAnimationKeyframeSnapPoints(
    const Timeline& timeline,
    const std::unordered_set<std::string>& excludeElementIds
) {
    std::vector<SnapPoint> snapPoints;

    for (const auto* track : timeline.allTracks()) {
        if (!track) continue;
        for (const auto& clip : track->clips()) {
            if (excludeElementIds.contains(clip.id().str())) {
                continue;
            }

            for (const auto& [prop, channel] : clip.animationChannels()) {
                for (const auto& kf : channel.keyframes()) {
                    snapPoints.push_back(SnapPoint{
                        .time = clip.startTime() + kf.time,
                        .type = SnapPointType::Keyframe,
                        .elementId = clip.id().str(),
                        .trackId = track->id().str()
                    });
                }
            }
        }
    }

    return snapPoints;
}

} // namespace catchim::editor::snapping
