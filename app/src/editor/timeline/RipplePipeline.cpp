#include "editor/timeline/RipplePipeline.h"

namespace catchim::editor {

size_t RipplePipeline::shiftClipsAfter(
    Timeline& timeline,
    core::TimelineTime cutoffTime,
    core::TimelineTime delta,
    const std::optional<core::TrackId>& trackId
) {
    if (delta.ticks() == 0) return 0;
    size_t count = 0;

    auto shiftOnTrack = [&](Track& track) {
        for (auto& clip : track.clips()) {
            if (clip.startTime() >= cutoffTime) {
                clip.setStartTime(clip.startTime() + delta);
                count++;
            }
        }
    };

    if (trackId.has_value()) {
        Track* track = timeline.findTrack(*trackId);
        if (track) shiftOnTrack(*track);
    } else {
        for (Track* track : timeline.allTracks()) {
            shiftOnTrack(*track);
        }
    }

    return count;
}

// RippleInsertCommand implementation
RippleInsertCommand::RippleInsertCommand(
    Timeline& timeline,
    core::TrackId trackId,
    Clip clip,
    bool rippleAllTracks
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clip_(std::move(clip))
    , rippleAllTracks_(rippleAllTracks)
    , insertedClipId_(clip_.id())
{
}

bool RippleInsertCommand::execute() {
    Track* track = timeline_.findTrack(trackId_);
    if (!track) return false;

    // Shift downstream clips right
    std::optional<core::TrackId> targetTrack = rippleAllTracks_ ? std::nullopt : std::optional<core::TrackId>(trackId_);
    RipplePipeline::shiftClipsAfter(timeline_, clip_.startTime(), clip_.duration(), targetTrack);

    // Add clip
    return timeline_.addClip(trackId_, clip_);
}

bool RippleInsertCommand::undo() {
    // Remove inserted clip
    auto removed = timeline_.removeClip(insertedClipId_);
    if (!removed.has_value()) return false;

    // Shift downstream clips left
    core::TimelineTime negativeDelta(-clip_.duration().ticks());
    std::optional<core::TrackId> targetTrack = rippleAllTracks_ ? std::nullopt : std::optional<core::TrackId>(trackId_);
    RipplePipeline::shiftClipsAfter(timeline_, clip_.startTime(), negativeDelta, targetTrack);

    return true;
}

// RippleMoveCommand implementation
RippleMoveCommand::RippleMoveCommand(
    Timeline& timeline,
    core::ClipId clipId,
    core::TimelineTime delta,
    bool rippleAllTracks
)
    : timeline_(timeline)
    , clipId_(std::move(clipId))
    , delta_(delta)
    , rippleAllTracks_(rippleAllTracks)
{
}

bool RippleMoveCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    if (!executed_) {
        oldStartTime_ = clip->startTime();
        executed_ = true;
    }

    Track* track = timeline_.findTrackContainingClip(clipId_);
    if (!track) return false;

    core::TimelineTime oldStart = clip->startTime();
    clip->setStartTime(oldStart + delta_);

    // Shift other downstream clips
    std::optional<core::TrackId> targetTrack = rippleAllTracks_ ? std::nullopt : std::optional<core::TrackId>(track->id());
    for (Track* t : timeline_.allTracks()) {
        if (!rippleAllTracks_ && t->id() != track->id()) continue;
        for (auto& other : t->clips()) {
            if (other.id() != clipId_ && other.startTime() >= oldStart) {
                other.setStartTime(other.startTime() + delta_);
            }
        }
    }

    return true;
}

bool RippleMoveCommand::undo() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    Track* track = timeline_.findTrackContainingClip(clipId_);
    if (!track) return false;

    core::TimelineTime currentStart = clip->startTime();
    clip->setStartTime(oldStartTime_);

    // Revert downstream shift
    core::TimelineTime negDelta(-delta_.ticks());
    for (Track* t : timeline_.allTracks()) {
        if (!rippleAllTracks_ && t->id() != track->id()) continue;
        for (auto& other : t->clips()) {
            if (other.id() != clipId_ && other.startTime() >= currentStart) {
                other.setStartTime(other.startTime() + negDelta);
            }
        }
    }

    return true;
}

} // namespace catchim::editor
