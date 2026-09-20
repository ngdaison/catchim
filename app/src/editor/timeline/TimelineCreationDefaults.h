#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/Track.h"
#include <optional>
#include <string>

namespace catchim::editor {

class TimelineCreationDefaults {
public:
    static constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
    static constexpr double TIMELINE_ZOOM_MIN = 0.1;
    static constexpr double TIMELINE_ZOOM_MAX = 100.0;

    static constexpr double VOLUME_DB_MIN = -60.0;
    static constexpr double VOLUME_DB_MAX = 20.0;

    static core::TimelineTime defaultNewElementDuration() noexcept;

    static core::TimelineTime toElementDurationTicks(
        std::optional<double> seconds
    ) noexcept;

    static std::string getDefaultTrackName(TrackType type);
    static std::string getDefaultTrackName(const std::string& typeStr);

    static bool isVolumeDbValid(double volumeDb) noexcept;
    static double clampVolumeDb(double volumeDb) noexcept;

    static bool isZoomLevelValid(double zoomLevel) noexcept;
    static double clampZoomLevel(double zoomLevel) noexcept;
};

} // namespace catchim::editor
