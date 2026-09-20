#include "TrackCommands.h"
#include <algorithm>

namespace catchim::editor {

// ==========================================
// AddTrackCommand
// ==========================================

AddTrackCommand::AddTrackCommand(
    Timeline& timeline,
    TrackType type,
    std::string name,
    std::optional<size_t> index
)
    : timeline_(timeline)
    , type_(type)
    , trackName_(std::move(name))
    , index_(index)
    , trackId_(core::TrackId::generate())
{
}

bool AddTrackCommand::execute() {
    timeline_.insertTrack(type_, trackName_, index_, trackId_);
    executed_ = true;
    return true;
}

bool AddTrackCommand::undo() {
    if (!executed_) return false;
    timeline_.removeTrack(trackId_);
    executed_ = false;
    return true;
}

// ==========================================
// RemoveTrackCommand
// ==========================================

RemoveTrackCommand::RemoveTrackCommand(Timeline& timeline, core::TrackId trackId)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
{
}

bool RemoveTrackCommand::execute() {
    // Determine original index before extraction
    originalIndex_ = std::nullopt;
    for (size_t i = 0; i < timeline_.overlayTracks().size(); ++i) {
        if (timeline_.overlayTracks()[i].id() == trackId_) {
            originalIndex_ = i;
            break;
        }
    }
    if (!originalIndex_.has_value()) {
        for (size_t i = 0; i < timeline_.audioTracks().size(); ++i) {
            if (timeline_.audioTracks()[i].id() == trackId_) {
                originalIndex_ = i;
                break;
            }
        }
    }

    auto extracted = timeline_.extractTrack(trackId_);
    if (!extracted.has_value()) {
        return false;
    }

    removedTrack_ = std::move(extracted);
    return true;
}

bool RemoveTrackCommand::undo() {
    if (!removedTrack_.has_value()) {
        return false;
    }

    auto& restored = timeline_.insertTrack(
        removedTrack_->type(),
        removedTrack_->name(),
        originalIndex_,
        removedTrack_->id()
    );
    restored.setMuted(removedTrack_->isMuted());
    restored.setHidden(removedTrack_->isHidden());
    restored.clips() = removedTrack_->clips();

    return true;
}

// ==========================================
// ToggleTrackMuteCommand
// ==========================================

ToggleTrackMuteCommand::ToggleTrackMuteCommand(Timeline& timeline, core::TrackId trackId)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
{
}

bool ToggleTrackMuteCommand::execute() {
    Track* track = timeline_.findTrack(trackId_);
    if (!track) return false;

    if (!initialized_) {
        prevMuted_ = track->isMuted();
        initialized_ = true;
    }

    track->setMuted(!prevMuted_);
    return true;
}

bool ToggleTrackMuteCommand::undo() {
    Track* track = timeline_.findTrack(trackId_);
    if (!track) return false;

    track->setMuted(prevMuted_);
    return true;
}

// ==========================================
// ToggleTrackVisibilityCommand
// ==========================================

ToggleTrackVisibilityCommand::ToggleTrackVisibilityCommand(Timeline& timeline, core::TrackId trackId)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
{
}

bool ToggleTrackVisibilityCommand::execute() {
    Track* track = timeline_.findTrack(trackId_);
    if (!track) return false;

    if (!initialized_) {
        prevHidden_ = track->isHidden();
        initialized_ = true;
    }

    track->setHidden(!prevHidden_);
    return true;
}

bool ToggleTrackVisibilityCommand::undo() {
    Track* track = timeline_.findTrack(trackId_);
    if (!track) return false;

    track->setHidden(prevHidden_);
    return true;
}

} // namespace catchim::editor
