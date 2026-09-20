#pragma once

#include "Track.h"
#include "TimelineSnappingEngine.h"
#include "core/time/TimelineTime.h"
#include <string>
#include <vector>

namespace catchim::editor {

class TimelineTrackDefaults {
public:
    static constexpr double VOLUME_DB_MIN = -60.0;
    static constexpr double VOLUME_DB_MAX = 20.0;

    static std::string getDefaultTrackName(TrackType type);

    static double clampVolumeDb(double db) noexcept;

    static std::vector<snapping::TimelineSnapPoint> getPlayheadSnapPoints(
        core::TimelineTime playheadTime
    );
};

} // namespace catchim::editor
