#pragma once

#include "editor/timeline/Timeline.h"
#include "editor/timeline/Track.h"
#include "editor/timeline/Clip.h"
#include "core/ids/Ids.h"
#include <functional>
#include <optional>

namespace catchim::editor {

class TrackCapabilityEngine {
public:
    using ClipUpdater = std::function<void(Clip&)>;
    using ClipPredicate = std::function<bool(const Clip&)>;

    // Capability queries mirroring web/src/timeline/track-capabilities.ts
    static bool canTrackHaveAudio(TrackType type) noexcept;
    static bool canTrackBeHidden(TrackType type) noexcept;

    // Hierarchy queries & updates mirroring web/src/timeline/track-element-update.ts
    static Track* findTrackInTimeline(Timeline& timeline, const core::TrackId& trackId) noexcept;
    static const Track* findTrackInTimeline(const Timeline& timeline, const core::TrackId& trackId) noexcept;

    static bool updateClipInTrack(
        Track& track,
        const core::ClipId& clipId,
        const ClipUpdater& updater,
        const ClipPredicate& predicate = nullptr
    );

    static bool updateClipInTimeline(
        Timeline& timeline,
        const core::ClipId& clipId,
        const ClipUpdater& updater,
        const ClipPredicate& predicate = nullptr
    );

    static bool setTrackHidden(
        Timeline& timeline,
        const core::TrackId& trackId,
        bool hidden
    );

    static bool setTrackMuted(
        Timeline& timeline,
        const core::TrackId& trackId,
        bool muted
    );
};

} // namespace catchim::editor
