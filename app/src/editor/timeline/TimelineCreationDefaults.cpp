#include "TimelineCreationDefaults.h"
#include <algorithm>

namespace catchim::editor {

core::TimelineTime TimelineCreationDefaults::defaultNewElementDuration() noexcept {
    return core::TimelineTime::fromSeconds(5.0);
}

core::TimelineTime TimelineCreationDefaults::toElementDurationTicks(
    std::optional<double> seconds
) noexcept {
    if (!seconds.has_value()) {
        return defaultNewElementDuration();
    }
    return core::TimelineTime::fromSeconds(*seconds);
}

std::string TimelineCreationDefaults::getDefaultTrackName(TrackType type) {
    switch (type) {
    case TrackType::Video:
        return "Video track";
    case TrackType::Audio:
        return "Audio track";
    case TrackType::Text:
        return "Text track";
    case TrackType::Graphic:
        return "Graphic track";
    case TrackType::Effect:
        return "Effect track";
    }
    return "Track";
}

std::string TimelineCreationDefaults::getDefaultTrackName(const std::string& typeStr) {
    if (typeStr == "video") return "Video track";
    if (typeStr == "audio") return "Audio track";
    if (typeStr == "text") return "Text track";
    if (typeStr == "graphic" || typeStr == "overlay") return "Graphic track";
    if (typeStr == "effect") return "Effect track";
    return "Track";
}

bool TimelineCreationDefaults::isVolumeDbValid(double volumeDb) noexcept {
    return volumeDb >= VOLUME_DB_MIN && volumeDb <= VOLUME_DB_MAX;
}

double TimelineCreationDefaults::clampVolumeDb(double volumeDb) noexcept {
    return std::clamp(volumeDb, VOLUME_DB_MIN, VOLUME_DB_MAX);
}

bool TimelineCreationDefaults::isZoomLevelValid(double zoomLevel) noexcept {
    return zoomLevel >= TIMELINE_ZOOM_MIN && zoomLevel <= TIMELINE_ZOOM_MAX;
}

double TimelineCreationDefaults::clampZoomLevel(double zoomLevel) noexcept {
    return std::clamp(zoomLevel, TIMELINE_ZOOM_MIN, TIMELINE_ZOOM_MAX);
}

} // namespace catchim::editor
