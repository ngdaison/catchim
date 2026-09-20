#pragma once

#include "core/time/TimelineTime.h"
#include "core/ids/Ids.h"
#include "editor/timeline/Timeline.h"
#include "editor/history/Command.h"
#include <vector>
#include <string>
#include <optional>

namespace catchim::editor {

struct GroupMember {
    core::TrackId trackId;
    core::ClipId clipId;
    ClipType clipType{ClipType::Video};
    core::TimelineTime originalStartTime{0};
    core::TimelineTime duration{0};
    core::TimelineTime timeOffset{0}; // relative to anchor clip original start time
};

struct MoveGroup {
    GroupMember anchor;
    std::vector<GroupMember> members;
};

struct PlannedClipMove {
    core::TrackId sourceTrackId;
    core::TrackId targetTrackId;
    core::ClipId clipId;
    core::TimelineTime newStartTime;
};

struct GroupMovePlan {
    std::vector<PlannedClipMove> moves;
    core::TimelineTime clampedAnchorTime;
};

class GroupMoveEngine {
public:
    static std::optional<MoveGroup> buildMoveGroup(
        const Timeline& timeline,
        const core::TrackId& anchorTrackId,
        const core::ClipId& anchorClipId,
        const std::vector<std::pair<core::TrackId, core::ClipId>>& selectedElements
    );

    static GroupMovePlan resolveGroupMove(
        const MoveGroup& group,
        core::TimelineTime proposedAnchorTime
    );
};

class GroupMoveCommand : public EditorCommand {
public:
    GroupMoveCommand(Timeline& timeline, GroupMovePlan plan);

    bool execute() override;
    bool undo() override;
    std::string name() const override;

private:
    Timeline& timeline_;
    GroupMovePlan plan_;
    struct OriginalState {
        core::TrackId trackId;
        core::ClipId clipId;
        core::TimelineTime startTime;
    };
    std::vector<OriginalState> originalStates_;
};

} // namespace catchim::editor
