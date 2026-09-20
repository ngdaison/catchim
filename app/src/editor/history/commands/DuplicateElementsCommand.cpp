#include "DuplicateElementsCommand.h"
#include <unordered_map>
#include <set>

namespace catchim::editor {

DuplicateElementsCommand::DuplicateElementsCommand(
    Timeline& timeline,
    std::vector<ElementLocation> elements
)
    : timeline_(timeline)
    , elementsToDuplicate_(std::move(elements))
{
}

bool DuplicateElementsCommand::execute() {
    if (elementsToDuplicate_.empty()) {
        return true;
    }

    savedSnapshot_ = timeline_.createSnapshot();
    duplicatedElements_.clear();

    // Group elements by track
    std::unordered_map<core::TrackId, std::vector<core::ClipId>> grouped;
    for (const auto& loc : elementsToDuplicate_) {
        grouped[loc.trackId].push_back(loc.clipId);
    }

    for (const auto& [tid, clipIds] : grouped) {
        Track* origTrack = timeline_.findTrack(tid);
        if (!origTrack) continue;

        TrackType trackType = origTrack->type();
        std::string newTrackName = origTrack->name() + " (copy)";

        std::set<core::ClipId> targetClipIds(clipIds.begin(), clipIds.end());
        std::vector<Clip> clonedClips;
        for (const auto& clip : origTrack->clips()) {
            if (targetClipIds.find(clip.id()) != targetClipIds.end()) {
                core::ClipId newId = core::ClipId::generate();
                Clip cloned = clip.clone(newId);
                cloned.setName(clip.name() + " (copy)");
                clonedClips.push_back(std::move(cloned));
            }
        }

        if (clonedClips.empty()) {
            continue;
        }

        Track& newTrack = timeline_.addTrack(trackType, newTrackName);
        for (auto& cloned : clonedClips) {
            core::ClipId newId = cloned.id();
            newTrack.insertClip(std::move(cloned));
            duplicatedElements_.push_back(ElementLocation{newTrack.id(), newId});
        }
    }

    return !duplicatedElements_.empty();
}

bool DuplicateElementsCommand::undo() {
    if (!savedSnapshot_.has_value()) {
        return false;
    }

    timeline_.restoreSnapshot(*savedSnapshot_);
    return true;
}

} // namespace catchim::editor
