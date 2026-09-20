#pragma once

#include "editor/timeline/Timeline.h"
#include <functional>
#include <vector>
#include <map>
#include <cstddef>

namespace catchim::editor {

struct PruneResult {
    size_t overlayTracksPruned{0};
    size_t audioTracksPruned{0};
    bool changed{false};

    bool operator==(const PruneResult& other) const noexcept {
        return overlayTracksPruned == other.overlayTracksPruned &&
               audioTracksPruned == other.audioTracksPruned &&
               changed == other.changed;
    }
};

class CommandReactorPipeline {
public:
    using ReactorCallback = std::function<void()>;

    CommandReactorPipeline() = default;

    size_t registerReactor(ReactorCallback callback);
    bool unregisterReactor(size_t tokenId);
    void notifyReactors();
    size_t reactorCount() const noexcept;
    void clear();

    static PruneResult pruneEmptyTracks(Timeline& timeline);

    static ReactorCallback createAutoPruneReactor(
        Timeline& timeline,
        std::function<void(const PruneResult&)> onPruned = nullptr
    );

private:
    size_t nextTokenId_{1};
    std::map<size_t, ReactorCallback> reactors_;
};

} // namespace catchim::editor
