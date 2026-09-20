#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include <string>
#include <optional>
#include <memory>

namespace catchim::editor {

/**
 * @brief Command to add a new track to the timeline at an optional index.
 * Corresponds to web/src/commands/timeline/track/add-track.ts.
 */
class AddTrackCommand : public EditorCommand {
public:
    AddTrackCommand(
        Timeline& timeline,
        TrackType type,
        std::string name,
        std::optional<size_t> index = std::nullopt
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Add Track"; }

    const core::TrackId& trackId() const noexcept { return trackId_; }

private:
    Timeline& timeline_;
    TrackType type_;
    std::string trackName_;
    std::optional<size_t> index_;
    core::TrackId trackId_;
    bool executed_{false};
};

/**
 * @brief Command to remove an existing track from the timeline.
 * Corresponds to web/src/commands/timeline/track/remove-track.ts.
 */
class RemoveTrackCommand : public EditorCommand {
public:
    RemoveTrackCommand(Timeline& timeline, core::TrackId trackId);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Remove Track"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    std::optional<Track> removedTrack_{std::nullopt};
    std::optional<size_t> originalIndex_{std::nullopt};
};

/**
 * @brief Command to toggle the mute state of a track.
 * Corresponds to web/src/commands/timeline/track/toggle-track-mute.ts.
 */
class ToggleTrackMuteCommand : public EditorCommand {
public:
    ToggleTrackMuteCommand(Timeline& timeline, core::TrackId trackId);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Toggle Track Mute"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    bool prevMuted_{false};
    bool initialized_{false};
};

/**
 * @brief Command to toggle the visibility (hidden flag) of a track.
 * Corresponds to web/src/commands/timeline/track/toggle-track-visibility.ts.
 */
class ToggleTrackVisibilityCommand : public EditorCommand {
public:
    ToggleTrackVisibilityCommand(Timeline& timeline, core::TrackId trackId);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Toggle Track Visibility"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    bool prevHidden_{false};
    bool initialized_{false};
};

} // namespace catchim::editor
