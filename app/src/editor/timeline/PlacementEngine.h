#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include "editor/timeline/Clip.h"
#include "editor/timeline/Track.h"
#include <vector>
#include <optional>
#include <string>

namespace catchim::editor {

class Timeline;

struct PlacementTimeSpan {
    core::TimelineTime startTime{0};
    core::TimelineTime duration{0};
    std::optional<core::ClipId> excludeClipId{std::nullopt};
};

struct TrackPlacementResult {
    enum class Kind {
        ExistingTrack,
        NewTrack
    } kind{Kind::ExistingTrack};

    size_t trackIndex{0};
    core::TrackId trackId{core::TrackId::empty()};
    TrackType trackType{TrackType::Video};
    size_t insertIndex{0};
    std::string insertPosition{"below"}; // "above" or "below"
};

class PlacementEngine {
public:
    // Fast collision checks
    static bool canPlaceTimeSpansOnTrack(
        const Track& track,
        const std::vector<PlacementTimeSpan>& timeSpans
    ) noexcept;

    static bool canPlaceClipOnTrack(
        const Track& track,
        core::TimelineTime startTime,
        core::TimelineTime duration,
        const std::optional<core::ClipId>& excludeClipId = std::nullopt
    ) noexcept;

    // Scan for earliest available gap on a track
    static core::TimelineTime findAvailableGap(
        const Track& track,
        core::TimelineTime minStartTime,
        core::TimelineTime duration
    ) noexcept;

    // Magnetic Main Track start enforcement (no gap allowed before first clip)
    static core::TimelineTime enforceMainTrackStart(
        const Track& mainTrack,
        core::TimelineTime requestedStartTime,
        const std::optional<core::ClipId>& excludeClipId = std::nullopt
    ) noexcept;

    // Compatibility validation
    static TrackType getTrackTypeForClipType(ClipType clipType) noexcept;
    static bool canClipGoOnTrack(ClipType clipType, TrackType trackType) noexcept;

    // Preferred track placement for inserting new elements/tracks
    static size_t getDefaultInsertIndexForTrack(
        const Timeline& timeline,
        TrackType trackType
    ) noexcept;

    static TrackPlacementResult resolvePreferredTrackPlacement(
        const Timeline& timeline,
        TrackType trackType,
        size_t preferredIndex,
        const std::string& direction = "below"
    ) noexcept;
};

} // namespace catchim::editor
