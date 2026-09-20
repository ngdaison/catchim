#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include "core/time/RationalFrameRate.h"
#include "editor/history/Command.h"
#include <vector>
#include <optional>
#include <memory>
#include <string>

namespace catchim::editor {

class Timeline;

enum class ResizeSide {
    Left,
    Right
};

struct GroupResizeMember {
    core::TrackId trackId;
    core::ClipId elementId;
    core::TimelineTime startTime;
    core::TimelineTime duration;
    core::TimelineTime trimStart{0};
    core::TimelineTime trimEnd{0};
    std::optional<core::TimelineTime> sourceDuration{std::nullopt};
    double retimeRate{1.0};
    std::optional<core::TimelineTime> leftNeighborBound{std::nullopt};
    std::optional<core::TimelineTime> rightNeighborBound{std::nullopt};
};

struct GroupResizePatch {
    core::TimelineTime trimStart{0};
    core::TimelineTime trimEnd{0};
    core::TimelineTime startTime{0};
    core::TimelineTime duration{0};
};

struct GroupResizeUpdate {
    core::TrackId trackId;
    core::ClipId elementId;
    GroupResizePatch patch;
};

struct GroupResizeResult {
    core::TimelineTime deltaTime{0};
    std::vector<GroupResizeUpdate> updates;
};

class GroupResizeEngine {
public:
    static GroupResizeResult computeGroupResize(
        const std::vector<GroupResizeMember>& members,
        ResizeSide side,
        core::TimelineTime deltaTime,
        const core::FrameRate& fps
    ) noexcept;

    static std::vector<GroupResizeMember> buildResizeMembers(
        const Timeline& timeline,
        const std::vector<core::ClipId>& clipIds
    );
};

class GroupResizeCommand : public EditorCommand {
public:
    GroupResizeCommand(
        Timeline& timeline,
        std::vector<GroupResizeUpdate> updates,
        std::string name = "Group Resize Clips"
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return name_; }

private:
    Timeline& timeline_;
    std::vector<GroupResizeUpdate> updates_;
    std::string name_;

    struct PreviousClipState {
        core::TrackId trackId;
        core::ClipId elementId;
        core::TimelineTime startTime;
        core::TimelineTime duration;
        core::TimelineTime trimStart;
        core::TimelineTime trimEnd;
    };
    std::vector<PreviousClipState> previousStates_;
};

} // namespace catchim::editor
