#pragma once

#include "editor/timeline/Track.h"
#include <vector>
#include <functional>

namespace catchim::editor {

class TrackLayoutMetrics {
public:
    static constexpr double TRACK_HEIGHT_VIDEO = 65.0;
    static constexpr double TRACK_HEIGHT_AUDIO = 50.0;
    static constexpr double TRACK_HEIGHT_TEXT = 25.0;
    static constexpr double TRACK_HEIGHT_GRAPHIC = 25.0;
    static constexpr double TRACK_HEIGHT_EFFECT = 25.0;

    static constexpr double KEYFRAME_LANE_HEIGHT = 20.0;
    static constexpr double TIMELINE_TRACK_GAP = 6.0;
    static constexpr double TIMELINE_CONTENT_TOP_PADDING = 2.0;
    static constexpr double TIMELINE_TRACK_LABELS_COLUMN_WIDTH = 112.0;
    static constexpr double TIMELINE_RULER_HEIGHT = 22.0;

    static double getTrackHeight(TrackType type) noexcept;
    static double getExpandedTrackHeight(TrackType type, size_t expandedLaneCount) noexcept;

    static double getCumulativeHeightBefore(
        const std::vector<const Track*>& tracks,
        size_t trackIndex,
        const std::function<double(size_t)>& getExtraHeight = nullptr
    ) noexcept;

    static std::vector<double> getTrackLayoutOffsets(
        const std::vector<const Track*>& tracks,
        const std::function<double(size_t)>& getExtraHeight = nullptr
    ) noexcept;

    static double getTotalTracksHeight(
        const std::vector<const Track*>& tracks,
        const std::function<double(size_t)>& getExtraHeight = nullptr
    ) noexcept;
};

} // namespace catchim::editor
