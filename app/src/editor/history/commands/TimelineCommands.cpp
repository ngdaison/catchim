#include "TimelineCommands.h"

namespace catchim::editor {

// --- AddClipCommand ---
AddClipCommand::AddClipCommand(Timeline& timeline, core::TrackId trackId, Clip clip)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clip_(std::move(clip))
{
}

bool AddClipCommand::execute() {
    return timeline_.addClip(trackId_, clip_);
}

bool AddClipCommand::undo() {
    auto removed = timeline_.removeClip(clip_.id());
    return removed.has_value();
}

// --- DeleteClipCommand ---
DeleteClipCommand::DeleteClipCommand(Timeline& timeline, core::ClipId clipId)
    : timeline_(timeline)
    , clipId_(std::move(clipId))
{
}

bool DeleteClipCommand::execute() {
    Track* track = timeline_.findTrackContainingClip(clipId_);
    if (!track) return false;

    originalTrackId_ = track->id();
    removedClip_ = timeline_.removeClip(clipId_);
    return removedClip_.has_value();
}

bool DeleteClipCommand::undo() {
    if (!removedClip_.has_value() || !originalTrackId_.has_value()) return false;
    return timeline_.addClip(*originalTrackId_, *removedClip_);
}

// --- MoveClipCommand ---
MoveClipCommand::MoveClipCommand(
    Timeline& timeline,
    core::ClipId clipId,
    core::TrackId targetTrackId,
    core::TimelineTime targetTime
)
    : timeline_(timeline)
    , clipId_(std::move(clipId))
    , targetTrackId_(std::move(targetTrackId))
    , targetTime_(targetTime)
{
}

bool MoveClipCommand::execute() {
    if (!initialized_) {
        Track* track = timeline_.findTrackContainingClip(clipId_);
        if (!track) return false;
        const Clip* clip = track->findClip(clipId_);
        if (!clip) return false;
        prevTrackId_ = track->id();
        prevTime_ = clip->startTime();
        initialized_ = true;
    }
    return timeline_.moveClip(clipId_, targetTrackId_, targetTime_);
}

bool MoveClipCommand::undo() {
    if (!initialized_) return false;
    return timeline_.moveClip(clipId_, prevTrackId_, prevTime_);
}

// --- TrimClipCommand ---
TrimClipCommand::TrimClipCommand(
    Timeline& timeline,
    core::ClipId clipId,
    Edge edge,
    core::TimelineTime newTime
)
    : timeline_(timeline)
    , clipId_(std::move(clipId))
    , edge_(edge)
    , newTime_(newTime)
{
}

bool TrimClipCommand::execute() {
    const Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    if (!initialized_) {
        prevStartTime_ = clip->startTime();
        prevDuration_ = clip->duration();
        prevTrimStart_ = clip->trimStart();
        initialized_ = true;
    }

    if (edge_ == Edge::Start) {
        return timeline_.trimClipStart(clipId_, newTime_);
    } else {
        return timeline_.trimClipEnd(clipId_, newTime_);
    }
}

bool TrimClipCommand::undo() {
    if (!initialized_) return false;
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    clip->setStartTime(prevStartTime_);
    clip->setDuration(prevDuration_);
    clip->setTrimStart(prevTrimStart_);
    Track* track = timeline_.findTrackContainingClip(clipId_);
    if (track) track->sortClips();
    return true;
}

// --- SplitClipCommand ---
SplitClipCommand::SplitClipCommand(Timeline& timeline, core::ClipId clipId, core::TimelineTime splitTime)
    : timeline_(timeline)
    , clipId_(std::move(clipId))
    , splitTime_(splitTime)
{
}

bool SplitClipCommand::execute() {
    const Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;
    prevDuration_ = clip->duration();

    auto [leftId, rightId] = timeline_.splitClip(clipId_, splitTime_);
    if (rightId.has_value()) {
        createdRightClipId_ = rightId;
        return true;
    }
    return false;
}

bool SplitClipCommand::undo() {
    if (!createdRightClipId_.has_value()) return false;

    // Remove right clip
    timeline_.removeClip(*createdRightClipId_);

    // Restore left clip duration
    Clip* leftClip = timeline_.findClip(clipId_);
    if (leftClip) {
        leftClip->setDuration(prevDuration_);
        return true;
    }
    return false;
}

} // namespace catchim::editor
