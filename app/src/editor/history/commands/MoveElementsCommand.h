#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/GroupMoveEngine.h"
#include <vector>
#include <string>
#include <optional>

namespace catchim::editor {

struct PlannedTrackCreation {
    core::TrackId id;
    TrackType type{TrackType::Video};
    size_t index{0};
};

/**
 * @brief Command to atomically move multiple elements across tracks with optional dynamic track creation.
 * Corresponds to web/src/commands/timeline/element/move-elements.ts.
 */
class MoveElementsCommand : public EditorCommand {
public:
    MoveElementsCommand(
        Timeline& timeline,
        std::vector<PlannedClipMove> moves,
        std::vector<PlannedTrackCreation> createTracks = {}
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Move Elements"; }

    const std::vector<PlannedClipMove>& moves() const noexcept { return moves_; }
    const std::vector<PlannedTrackCreation>& createTracks() const noexcept { return createTracks_; }

private:
    Timeline& timeline_;
    std::vector<PlannedClipMove> moves_;
    std::vector<PlannedTrackCreation> createTracks_;
    std::optional<TimelineTracksSnapshot> savedState_{std::nullopt};
};

} // namespace catchim::editor
