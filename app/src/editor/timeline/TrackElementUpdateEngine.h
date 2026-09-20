#pragma once

#include "core/ids/Ids.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/Clip.h"
#include <functional>
#include <optional>

namespace catchim::editor {

class TrackElementUpdateEngine {
public:
    using TrackUpdater = std::function<void(Track&)>;
    using ClipUpdater = std::function<void(Clip&)>;
    using ClipPredicate = std::function<bool(const Clip&)>;

    static Track* findTrackInTimeline(
        Timeline& timeline,
        const core::TrackId& trackId
    ) noexcept;

    static const Track* findTrackInTimeline(
        const Timeline& timeline,
        const core::TrackId& trackId
    ) noexcept;

    static bool updateTrackInTimeline(
        Timeline& timeline,
        const core::TrackId& trackId,
        const TrackUpdater& updater
    );

    static bool updateElementInTrack(
        Track& track,
        const core::ClipId& elementId,
        const ClipUpdater& updater,
        const std::optional<ClipPredicate>& predicate = std::nullopt
    );

    static bool updateElementInTimeline(
        Timeline& timeline,
        const core::TrackId& trackId,
        const core::ClipId& elementId,
        const ClipUpdater& updater,
        const std::optional<ClipPredicate>& predicate = std::nullopt
    );
};

} // namespace catchim::editor
