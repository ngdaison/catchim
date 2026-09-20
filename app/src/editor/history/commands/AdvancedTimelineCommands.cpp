#include "editor/history/commands/AdvancedTimelineCommands.h"
#include "core/logging/Logger.h"

namespace catchim::editor {

// ==================== DuplicateClipCommand ====================

DuplicateClipCommand::DuplicateClipCommand(
    Timeline& timeline,
    core::ClipId sourceClipId,
    std::optional<core::TimelineTime> targetStartTime
) : m_timeline(timeline),
    m_sourceClipId(std::move(sourceClipId)),
    m_targetStartTime(targetStartTime),
    m_duplicatedClipId(core::ClipId::generate()) {}

bool DuplicateClipCommand::execute() {
    const Clip* srcClip = m_timeline.findClip(m_sourceClipId);
    if (!srcClip) {
        LOG_ERROR("DuplicateClipCommand failed: source clip {} not found", m_sourceClipId.str());
        return false;
    }

    Track* track = m_timeline.findTrackContainingClip(m_sourceClipId);
    if (!track) {
        LOG_ERROR("DuplicateClipCommand failed: track containing source clip not found");
        return false;
    }

    m_trackId = track->id();

    core::TimelineTime newStart = m_targetStartTime.value_or(srcClip->endTime());
    if (!track->canPlace(newStart, srcClip->duration())) {
        // If target slot is blocked, place at end of track
        newStart = track->maxEndTime();
    }

    Clip clone = srcClip->clone(m_duplicatedClipId);
    clone.setStartTime(newStart);
    m_duplicatedClip = clone;

    track->insertClip(std::move(clone));
    return true;
}

bool DuplicateClipCommand::undo() {
    auto removed = m_timeline.removeClip(m_duplicatedClipId);
    return removed.has_value();
}

// ==================== SplitLeftCommand ====================

SplitLeftCommand::SplitLeftCommand(
    Timeline& timeline,
    core::ClipId clipId,
    core::TimelineTime splitTime
) : m_timeline(timeline),
    m_clipId(std::move(clipId)),
    m_splitTime(splitTime) {}

bool SplitLeftCommand::execute() {
    Clip* clip = m_timeline.findClip(m_clipId);
    if (!clip) return false;

    Track* track = m_timeline.findTrackContainingClip(m_clipId);
    if (!track) return false;
    m_trackId = track->id();

    m_originalClip = *clip;

    auto [leftId, rightId] = m_timeline.splitClip(m_clipId, m_splitTime);
    if (!leftId.has_value() || !rightId.has_value()) {
        return false;
    }

    m_rightClipId = *rightId;

    // Delete left portion
    auto removedLeft = m_timeline.removeClip(*leftId);
    return removedLeft.has_value();
}

bool SplitLeftCommand::undo() {
    // Remove right clip
    m_timeline.removeClip(m_rightClipId);

    // Restore original clip
    if (!m_originalClip.has_value()) return false;

    Track* track = m_timeline.findTrack(m_trackId);
    if (!track) return false;

    Clip restored = *m_originalClip;
    track->insertClip(std::move(restored));
    return true;
}

// ==================== SplitRightCommand ====================

SplitRightCommand::SplitRightCommand(
    Timeline& timeline,
    core::ClipId clipId,
    core::TimelineTime splitTime
) : m_timeline(timeline),
    m_clipId(std::move(clipId)),
    m_splitTime(splitTime) {}

bool SplitRightCommand::execute() {
    Clip* clip = m_timeline.findClip(m_clipId);
    if (!clip) return false;

    if (m_splitTime <= clip->startTime() || m_splitTime >= clip->endTime()) {
        return false;
    }

    m_originalDuration = clip->duration();
    core::TimelineTime newDuration = m_splitTime - clip->startTime();

    return m_timeline.trimClipEnd(m_clipId, newDuration);
}

bool SplitRightCommand::undo() {
    return m_timeline.trimClipEnd(m_clipId, m_originalDuration);
}

// ==================== RippleDeleteCommand ====================

RippleDeleteCommand::RippleDeleteCommand(
    Timeline& timeline,
    core::ClipId clipId
) : m_timeline(timeline),
    m_clipId(std::move(clipId)) {}

bool RippleDeleteCommand::execute() {
    Clip* clip = m_timeline.findClip(m_clipId);
    if (!clip) return false;

    Track* track = m_timeline.findTrackContainingClip(m_clipId);
    if (!track) return false;

    m_trackId = track->id();
    m_deletedStartTime = clip->startTime();
    m_deletedDuration = clip->duration();

    auto removed = m_timeline.removeClip(m_clipId);
    if (!removed.has_value()) return false;
    m_deletedClip = removed;

    // Shift subsequent clips left by deleted duration
    core::TimelineTime negDelta = core::TimelineTime::fromTicks(-m_deletedDuration.ticks());
    m_timeline.applyRipple(m_trackId, m_deletedStartTime, negDelta);
    return true;
}

bool RippleDeleteCommand::undo() {
    if (!m_deletedClip.has_value()) return false;
    Track* track = m_timeline.findTrack(m_trackId);
    if (!track) return false;

    // Shift clips back right
    m_timeline.applyRipple(m_trackId, m_deletedStartTime, m_deletedDuration);

    // Re-insert clip
    Clip restored = *m_deletedClip;
    track->insertClip(std::move(restored));
    return true;
}

} // namespace catchim::editor
