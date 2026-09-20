#include "editor/timeline/TrackCapabilityEngine.h"

namespace catchim::editor {

bool TrackCapabilityEngine::canTrackHaveAudio(TrackType type) noexcept {
    return type == TrackType::Audio || type == TrackType::Video;
}

bool TrackCapabilityEngine::canTrackBeHidden(TrackType type) noexcept {
    return type != TrackType::Audio;
}

Track* TrackCapabilityEngine::findTrackInTimeline(Timeline& timeline, const core::TrackId& trackId) noexcept {
    return timeline.findTrack(trackId);
}

const Track* TrackCapabilityEngine::findTrackInTimeline(const Timeline& timeline, const core::TrackId& trackId) noexcept {
    return timeline.findTrack(trackId);
}

bool TrackCapabilityEngine::updateClipInTrack(
    Track& track,
    const core::ClipId& clipId,
    const ClipUpdater& updater,
    const ClipPredicate& predicate
) {
    auto* clip = track.findClip(clipId);
    if (!clip) return false;

    if (predicate && !predicate(*clip)) {
        return false;
    }

    if (updater) {
        updater(*clip);
        return true;
    }
    return false;
}

bool TrackCapabilityEngine::updateClipInTimeline(
    Timeline& timeline,
    const core::ClipId& clipId,
    const ClipUpdater& updater,
    const ClipPredicate& predicate
) {
    auto* track = timeline.findTrackContainingClip(clipId);
    if (!track) return false;

    return updateClipInTrack(*track, clipId, updater, predicate);
}

bool TrackCapabilityEngine::setTrackHidden(
    Timeline& timeline,
    const core::TrackId& trackId,
    bool hidden
) {
    auto* track = timeline.findTrack(trackId);
    if (!track) return false;

    if (!canTrackBeHidden(track->type())) {
        return false; // Audio tracks cannot be hidden
    }

    track->setHidden(hidden);
    return true;
}

bool TrackCapabilityEngine::setTrackMuted(
    Timeline& timeline,
    const core::TrackId& trackId,
    bool muted
) {
    auto* track = timeline.findTrack(trackId);
    if (!track) return false;

    if (!canTrackHaveAudio(track->type())) {
        return false; // Tracks without audio capability cannot be muted
    }

    track->setMuted(muted);
    return true;
}

} // namespace catchim::editor
