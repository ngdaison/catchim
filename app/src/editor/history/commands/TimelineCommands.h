#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include <memory>

namespace catchim::editor {

class AddClipCommand : public EditorCommand {
public:
    AddClipCommand(Timeline& timeline, core::TrackId trackId, Clip clip);
    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Add Clip"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    Clip clip_;
};

class DeleteClipCommand : public EditorCommand {
public:
    DeleteClipCommand(Timeline& timeline, core::ClipId clipId);
    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Delete Clip"; }

private:
    Timeline& timeline_;
    core::ClipId clipId_;
    std::optional<core::TrackId> originalTrackId_{std::nullopt};
    std::optional<Clip> removedClip_{std::nullopt};
};

class MoveClipCommand : public EditorCommand {
public:
    MoveClipCommand(
        Timeline& timeline,
        core::ClipId clipId,
        core::TrackId targetTrackId,
        core::TimelineTime targetTime
    );
    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Move Clip"; }

private:
    Timeline& timeline_;
    core::ClipId clipId_;
    core::TrackId targetTrackId_;
    core::TimelineTime targetTime_;
    core::TrackId prevTrackId_;
    core::TimelineTime prevTime_;
    bool initialized_{false};
};

class TrimClipCommand : public EditorCommand {
public:
    enum class Edge { Start, End };
    TrimClipCommand(
        Timeline& timeline,
        core::ClipId clipId,
        Edge edge,
        core::TimelineTime newTime
    );
    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Trim Clip"; }

private:
    Timeline& timeline_;
    core::ClipId clipId_;
    Edge edge_;
    core::TimelineTime newTime_;
    core::TimelineTime prevStartTime_;
    core::TimelineTime prevDuration_;
    core::TimelineTime prevTrimStart_;
    bool initialized_{false};
};

class SplitClipCommand : public EditorCommand {
public:
    SplitClipCommand(Timeline& timeline, core::ClipId clipId, core::TimelineTime splitTime);
    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Split Clip"; }

private:
    Timeline& timeline_;
    core::ClipId clipId_;
    core::TimelineTime splitTime_;
    std::optional<core::ClipId> createdRightClipId_{std::nullopt};
    core::TimelineTime prevDuration_;
};

} // namespace catchim::editor
