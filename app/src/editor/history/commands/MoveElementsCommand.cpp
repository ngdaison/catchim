#include "editor/history/commands/MoveElementsCommand.h"
#include "editor/timeline/PlacementEngine.h"
#include <algorithm>

namespace catchim::editor {

MoveElementsCommand::MoveElementsCommand(
    Timeline& timeline,
    std::vector<PlannedClipMove> moves,
    std::vector<PlannedTrackCreation> createTracks
)
    : timeline_(timeline),
      moves_(std::move(moves)),
      createTracks_(std::move(createTracks)) {}

bool MoveElementsCommand::execute() {
    savedState_ = timeline_.createSnapshot();

    // 1. Create any planned new tracks
    auto sortedCreateTracks = createTracks_;
    std::sort(sortedCreateTracks.begin(), sortedCreateTracks.end(),
              [](const PlannedTrackCreation& a, const PlannedTrackCreation& b) {
                  return a.index < b.index;
              });

    for (const auto& ct : sortedCreateTracks) {
        if (!timeline_.findTrack(ct.id)) {
            std::string defaultName = (ct.type == TrackType::Audio) ? "Audio Track" : "Video Track";
            timeline_.insertTrack(ct.type, defaultName, ct.index, ct.id);
        }
    }

    // 2. Validate all moves before applying
    for (const auto& move : moves_) {
        Track* srcTrack = timeline_.findTrack(move.sourceTrackId);
        if (!srcTrack) return false;

        const Clip* clip = srcTrack->findClip(move.clipId);
        if (!clip) return false;

        Track* dstTrack = timeline_.findTrack(move.targetTrackId);
        if (!dstTrack) return false;

        if (!PlacementEngine::canClipGoOnTrack(clip->type(), dstTrack->type())) {
            return false;
        }
    }

    // 3. Perform moves
    for (const auto& move : moves_) {
        Track* srcTrack = timeline_.findTrack(move.sourceTrackId);
        auto optClip = srcTrack->removeClip(move.clipId);
        if (!optClip) continue;

        Clip movedClip = std::move(*optClip);
        movedClip.setStartTime(move.newStartTime);

        Track* dstTrack = timeline_.findTrack(move.targetTrackId);
        dstTrack->clips().push_back(std::move(movedClip));
    }

    // 4. Sort clips on all tracks
    for (auto* t : timeline_.allTracks()) {
        t->sortClips();
    }

    return true;
}

bool MoveElementsCommand::undo() {
    if (!savedState_) {
        return false;
    }
    timeline_.restoreSnapshot(*savedState_);
    return true;
}

} // namespace catchim::editor
