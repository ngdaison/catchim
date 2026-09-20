#include "TimelineTrackDefaults.h"
#include <algorithm>

namespace catchim::editor {

std::string TimelineTrackDefaults::getDefaultTrackName(TrackType type) {
    switch (type) {
        case TrackType::Video: return "Video track";
        case TrackType::Text: return "Text track";
        case TrackType::Audio: return "Audio track";
        case TrackType::Graphic: return "Graphic track";
        case TrackType::Effect: return "Effect track";
    }
    return "Video track";
}

double TimelineTrackDefaults::clampVolumeDb(double db) noexcept {
    return std::clamp(db, VOLUME_DB_MIN, VOLUME_DB_MAX);
}

std::vector<snapping::TimelineSnapPoint> TimelineTrackDefaults::getPlayheadSnapPoints(
    core::TimelineTime playheadTime
) {
    snapping::TimelineSnapPoint pt;
    pt.time = playheadTime;
    pt.type = snapping::TimelineSnapPointType::Playhead;
    return { pt };
}

} // namespace catchim::editor
