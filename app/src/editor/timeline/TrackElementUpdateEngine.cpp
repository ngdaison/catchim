#include "editor/timeline/TrackElementUpdateEngine.h"

namespace catchim::editor {

Track* TrackElementUpdateEngine::findTrackInTimeline(
    Timeline& timeline,
    const core::TrackId& trackId
) noexcept {
    return timeline.findTrack(trackId);
}

const Track* TrackElementUpdateEngine::findTrackInTimeline(
    const Timeline& timeline,
    const core::TrackId& trackId
) noexcept {
    return timeline.findTrack(trackId);
}

bool TrackElementUpdateEngine::updateTrackInTimeline(
    Timeline& timeline,
    const core::TrackId& trackId,
    const TrackUpdater& updater
) {
    Track* track = timeline.findTrack(trackId);
    if (!track) {
        return false;
    }
    updater(*track);
    return true;
}

bool TrackElementUpdateEngine::updateElementInTrack(
    Track& track,
    const core::ClipId& elementId,
    const ClipUpdater& updater,
    const std::optional<ClipPredicate>& predicate
) {
    for (auto& clip : track.clips()) {
        if (clip.id() == elementId) {
            if (predicate.has_value() && !(*predicate)(clip)) {
                return false;
            }
            updater(clip);
            return true;
        }
    }
    return false;
}

bool TrackElementUpdateEngine::updateElementInTimeline(
    Timeline& timeline,
    const core::TrackId& trackId,
    const core::ClipId& elementId,
    const ClipUpdater& updater,
    const std::optional<ClipPredicate>& predicate
) {
    Track* track = timeline.findTrack(trackId);
    if (!track) {
        return false;
    }
    return updateElementInTrack(*track, elementId, updater, predicate);
}

} // namespace catchim::editor
