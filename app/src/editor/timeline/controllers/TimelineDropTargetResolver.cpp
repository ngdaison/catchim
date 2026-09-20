#include "TimelineDropTargetResolver.h"
#include "editor/timeline/TimelineLayoutEngine.h"
#include "editor/timeline/PlacementEngine.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

namespace {
constexpr double TIMELINE_TRACK_GAP_PX = 6.0;
}

std::optional<DropTargetElement> TimelineDropTargetResolver::findElementAtPosition(
    double mouseX,
    const Timeline& timeline,
    size_t trackIndex,
    const std::vector<ClipType>& targetElementTypes,
    double pixelsPerSecond,
    double zoomLevel
) noexcept {
    const auto tracks = timeline.allTracks();
    if (trackIndex >= tracks.size()) {
        return std::nullopt;
    }

    const auto* track = tracks[trackIndex];
    if (!track) return std::nullopt;

    const double effectiveZoom = zoomLevel > 0.0 ? zoomLevel : 1.0;
    const double pps = pixelsPerSecond > 0.0 ? pixelsPerSecond : 50.0;
    const auto time = core::TimelineTime::fromSeconds(std::max(0.0, mouseX / (pps * effectiveZoom)));

    for (const auto& clip : track->clips()) {
        bool typeMatches = targetElementTypes.empty();
        for (const auto& t : targetElementTypes) {
            if (clip.type() == t) {
                typeMatches = true;
                break;
            }
        }

        if (typeMatches && clip.startTime() <= time && time < (clip.startTime() + clip.duration())) {
            return DropTargetElement{clip.id(), track->id()};
        }
    }

    return std::nullopt;
}

std::optional<TrackAtYResult> TimelineDropTargetResolver::getTrackAtY(
    double mouseY,
    const Timeline& timeline,
    const std::optional<std::string>& verticalDragDirection
) noexcept {
    double cumulativeHeight = 0.0;
    const auto tracks = timeline.allTracks();

    for (size_t i = 0; i < tracks.size(); ++i) {
        const auto* track = tracks[i];
        if (!track) continue;

        const double trackHeight = static_cast<double>(TimelineLayoutEngine::getTrackHeight(track->type()));
        const double trackTop = cumulativeHeight;
        const double trackBottom = trackTop + trackHeight;

        if (mouseY >= trackTop && mouseY < trackBottom) {
            return TrackAtYResult{i, mouseY - trackTop};
        }

        if (i + 1 < tracks.size() && verticalDragDirection.has_value()) {
            const double gapTop = trackBottom;
            const double gapBottom = gapTop + TIMELINE_TRACK_GAP_PX;
            if (mouseY >= gapTop && mouseY < gapBottom) {
                const bool isDraggingUp = (*verticalDragDirection == "up");
                return TrackAtYResult{
                    isDraggingUp ? i : i + 1,
                    isDraggingUp ? trackHeight - 1.0 : 0.0
                };
            }
        }

        cumulativeHeight += trackHeight + TIMELINE_TRACK_GAP_PX;
    }

    return std::nullopt;
}

DropTarget TimelineDropTargetResolver::computeDropTarget(
    const Timeline& timeline,
    const ComputeDropTargetParams& params
) {
    core::TimelineTime xPosition;
    if (params.startTimeOverride.has_value()) {
        xPosition = *params.startTimeOverride;
    } else if (params.isExternalDrop) {
        xPosition = params.playheadTime;
    } else {
        const double effectiveZoom = params.zoomLevel > 0.0 ? params.zoomLevel : 1.0;
        const double pps = params.pixelsPerSecond > 0.0 ? params.pixelsPerSecond : 50.0;
        const double seconds = std::max(0.0, params.mouseX / (pps * effectiveZoom));
        xPosition = core::TimelineTime::fromSeconds(seconds);
    }

    const auto tracks = timeline.allTracks();
    if (tracks.empty()) {
        return DropTarget{
            .trackIndex = 0,
            .isNewTrack = true,
            .insertPosition = std::nullopt,
            .xPosition = xPosition,
            .targetElement = std::nullopt
        };
    }

    const auto trackAtMouse = getTrackAtY(params.mouseY, timeline, params.verticalDragDirection);
    if (!trackAtMouse.has_value()) {
        const bool isAboveAll = (params.mouseY < 0.0);
        return DropTarget{
            .trackIndex = isAboveAll ? 0 : tracks.size(),
            .isNewTrack = true,
            .insertPosition = isAboveAll ? std::make_optional("above") : std::make_optional("below"),
            .xPosition = xPosition,
            .targetElement = std::nullopt
        };
    }

    const size_t trackIndex = trackAtMouse->trackIndex;
    const double relativeY = trackAtMouse->relativeY;

    if (!params.targetElementTypes.empty()) {
        auto targetElement = findElementAtPosition(
            params.mouseX,
            timeline,
            trackIndex,
            params.targetElementTypes,
            params.pixelsPerSecond,
            params.zoomLevel
        );
        if (targetElement.has_value()) {
            return DropTarget{
                .trackIndex = trackIndex,
                .isNewTrack = false,
                .insertPosition = std::nullopt,
                .xPosition = xPosition,
                .targetElement = targetElement
            };
        }
    }

    const auto* track = tracks[trackIndex];
    if (!track) {
        return DropTarget{
            .trackIndex = trackIndex,
            .isNewTrack = true,
            .insertPosition = std::make_optional("below"),
            .xPosition = xPosition,
            .targetElement = std::nullopt
        };
    }

    const double trackHeight = static_cast<double>(TimelineLayoutEngine::getTrackHeight(track->type()));
    const bool isAbove = relativeY < (trackHeight / 2.0);

    const bool canGo = PlacementEngine::canClipGoOnTrack(params.clipType, track->type());
    const bool canPlace = canGo && PlacementEngine::canPlaceClipOnTrack(*track, xPosition, params.elementDuration);

    if (canPlace) {
        return DropTarget{
            .trackIndex = trackIndex,
            .isNewTrack = false,
            .insertPosition = std::nullopt,
            .xPosition = xPosition,
            .targetElement = std::nullopt
        };
    }

    return DropTarget{
        .trackIndex = isAbove ? trackIndex : trackIndex + 1,
        .isNewTrack = true,
        .insertPosition = isAbove ? std::make_optional("above") : std::make_optional("below"),
        .xPosition = xPosition,
        .targetElement = std::nullopt
    };
}

double TimelineDropTargetResolver::getDropLineY(
    const DropTarget& dropTarget,
    const Timeline& timeline
) noexcept {
    const auto tracks = timeline.allTracks();
    const size_t safeIndex = std::min(dropTarget.trackIndex, tracks.size());
    double y = 0.0;

    for (size_t i = 0; i < safeIndex; ++i) {
        if (tracks[i]) {
            y += static_cast<double>(TimelineLayoutEngine::getTrackHeight(tracks[i]->type())) + TIMELINE_TRACK_GAP_PX;
        }
    }

    return y;
}

} // namespace catchim::editor
