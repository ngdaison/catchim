#pragma once

#include "editor/timeline/Timeline.h"
#include "editor/history/Command.h"
#include "core/time/TimelineTime.h"
#include <optional>
#include <string>

namespace catchim::editor {

class RipplePipeline {
public:
    static size_t shiftClipsAfter(
        Timeline& timeline,
        core::TimelineTime cutoffTime,
        core::TimelineTime delta,
        const std::optional<core::TrackId>& trackId = std::nullopt
    );
};

class RippleInsertCommand : public EditorCommand {
public:
    RippleInsertCommand(
        Timeline& timeline,
        core::TrackId trackId,
        Clip clip,
        bool rippleAllTracks = false
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Ripple Insert Clip"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    Clip clip_;
    bool rippleAllTracks_{false};
    core::ClipId insertedClipId_;
};

class RippleMoveCommand : public EditorCommand {
public:
    RippleMoveCommand(
        Timeline& timeline,
        core::ClipId clipId,
        core::TimelineTime delta,
        bool rippleAllTracks = false
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Ripple Move Clip"; }

private:
    Timeline& timeline_;
    core::ClipId clipId_;
    core::TimelineTime delta_;
    bool rippleAllTracks_{false};
    core::TimelineTime oldStartTime_{0};
    bool executed_{false};
};

} // namespace catchim::editor
