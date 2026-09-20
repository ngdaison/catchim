#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/Clip.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <memory>
#include <optional>

namespace catchim::editor {

// Duplicate a clip
class DuplicateClipCommand : public EditorCommand {
public:
    DuplicateClipCommand(
        Timeline& timeline,
        core::ClipId sourceClipId,
        std::optional<core::TimelineTime> targetStartTime = std::nullopt
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Duplicate Clip"; }

    core::ClipId duplicatedClipId() const { return m_duplicatedClipId; }

private:
    Timeline& m_timeline;
    core::ClipId m_sourceClipId;
    std::optional<core::TimelineTime> m_targetStartTime;
    core::ClipId m_duplicatedClipId;
    core::TrackId m_trackId;
    std::optional<Clip> m_duplicatedClip;
};

// Split and remove left portion (Shortcut Q)
class SplitLeftCommand : public EditorCommand {
public:
    SplitLeftCommand(Timeline& timeline, core::ClipId clipId, core::TimelineTime splitTime);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Split Left"; }

private:
    Timeline& m_timeline;
    core::ClipId m_clipId;
    core::TimelineTime m_splitTime;
    core::TrackId m_trackId;

    // Backup state for undo
    std::optional<Clip> m_originalClip;
    core::ClipId m_rightClipId;
};

// Split and remove right portion (Shortcut W)
class SplitRightCommand : public EditorCommand {
public:
    SplitRightCommand(Timeline& timeline, core::ClipId clipId, core::TimelineTime splitTime);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Split Right"; }

private:
    Timeline& m_timeline;
    core::ClipId m_clipId;
    core::TimelineTime m_splitTime;
    core::TimelineTime m_originalDuration;
};

// Ripple delete: deletes clip and closes the gap on the track
class RippleDeleteCommand : public EditorCommand {
public:
    RippleDeleteCommand(Timeline& timeline, core::ClipId clipId);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Ripple Delete Clip"; }

private:
    Timeline& m_timeline;
    core::ClipId m_clipId;
    core::TrackId m_trackId;
    std::optional<Clip> m_deletedClip;
    core::TimelineTime m_deletedStartTime{0};
    core::TimelineTime m_deletedDuration{0};
};

} // namespace catchim::editor
