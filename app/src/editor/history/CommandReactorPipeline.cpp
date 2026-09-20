#include "editor/history/CommandReactorPipeline.h"
#include <algorithm>

namespace catchim::editor {

size_t CommandReactorPipeline::registerReactor(ReactorCallback callback) {
    if (!callback) {
        return 0;
    }
    const size_t token = nextTokenId_++;
    reactors_[token] = std::move(callback);
    return token;
}

bool CommandReactorPipeline::unregisterReactor(size_t tokenId) {
    return reactors_.erase(tokenId) > 0;
}

void CommandReactorPipeline::notifyReactors() {
    // Copy callbacks in case a reactor modifies registrations during execution
    std::vector<ReactorCallback> callbacks;
    callbacks.reserve(reactors_.size());
    for (const auto& [id, cb] : reactors_) {
        callbacks.push_back(cb);
    }

    for (const auto& cb : callbacks) {
        if (cb) {
            cb();
        }
    }
}

size_t CommandReactorPipeline::reactorCount() const noexcept {
    return reactors_.size();
}

void CommandReactorPipeline::clear() {
    reactors_.clear();
}

PruneResult CommandReactorPipeline::pruneEmptyTracks(Timeline& timeline) {
    PruneResult result;

    // Prune overlay tracks
    auto& overlays = timeline.overlayTracks();
    const size_t initialOverlays = overlays.size();
    std::erase_if(overlays, [](const Track& trk) {
        return trk.clips().empty();
    });
    if (overlays.size() != initialOverlays) {
        result.overlayTracksPruned = initialOverlays - overlays.size();
        result.changed = true;
    }

    // Prune audio tracks
    auto& audios = timeline.audioTracks();
    const size_t initialAudios = audios.size();
    std::erase_if(audios, [](const Track& trk) {
        return trk.clips().empty();
    });
    if (audios.size() != initialAudios) {
        result.audioTracksPruned = initialAudios - audios.size();
        result.changed = true;
    }

    return result;
}

CommandReactorPipeline::ReactorCallback CommandReactorPipeline::createAutoPruneReactor(
    Timeline& timeline,
    std::function<void(const PruneResult&)> onPruned
) {
    return [&timeline, onPruned = std::move(onPruned)]() {
        const auto result = pruneEmptyTracks(timeline);
        if (result.changed && onPruned) {
            onPruned(result);
        }
    };
}

} // namespace catchim::editor
