#pragma once

#include "editor/timeline/Track.h"
#include "editor/timeline/Clip.h"
#include "core/ids/Ids.h"
#include <vector>
#include <string>
#include <unordered_set>

namespace catchim::editor {

struct TimelineLayoutMetrics {
    static constexpr int TRACK_HEIGHT_VIDEO = 65;
    static constexpr int TRACK_HEIGHT_AUDIO = 50;
    static constexpr int TRACK_HEIGHT_TEXT = 25;
    static constexpr int TRACK_HEIGHT_GRAPHIC = 25;
    static constexpr int TRACK_HEIGHT_EFFECT = 25;

    static constexpr int KEYFRAME_LANE_HEIGHT = 20;
    static constexpr int KEYFRAME_DIAMOND_SIZE = 14;
    static constexpr int EXPANDED_GROUP_HEADER_HEIGHT = 18;

    static constexpr int TIMELINE_TRACK_GAP = 6;
    static constexpr int TIMELINE_TRACK_LABELS_COLUMN_WIDTH = 112;
    static constexpr int TIMELINE_RULER_HEIGHT = 22;
    static constexpr int TIMELINE_BOOKMARK_ROW_HEIGHT = 16;
    static constexpr int TIMELINE_SCROLLBAR_SIZE = 12;
    static constexpr int TIMELINE_CONTENT_TOP_PADDING = 2;
};

struct ExpandedRow {
    std::string propertyPath;
    std::string label;
};

class TimelineLayoutEngine {
public:
    static int getTrackHeight(TrackType type) noexcept;
    static int getExpandedTrackHeight(TrackType type, size_t expandedLaneCount) noexcept;

    static int getCumulativeHeightBefore(
        const std::vector<TrackType>& tracks,
        size_t trackIndex,
        const std::vector<int>& extraHeights = {}
    ) noexcept;

    static std::vector<int> getTrackLayoutOffsets(
        const std::vector<TrackType>& tracks,
        const std::vector<int>& extraHeights = {}
    ) noexcept;

    static int getTotalTracksHeight(
        const std::vector<TrackType>& tracks,
        const std::vector<int>& extraHeights = {}
    ) noexcept;

    // Expanded Keyframe Rows
    static std::string getPropertyLabel(const std::string& path) noexcept;
    static std::vector<ExpandedRow> getExpandedRows(const Clip& clip);
    static int getExpansionHeight(const std::vector<ExpandedRow>& rows) noexcept;

    static int computeTrackExpansionHeight(
        const Track& track,
        const std::unordered_set<core::ClipId>& expandedClipIds
    );

    static std::vector<ExpandedRow> getTrackExpandedRows(
        const Track& track,
        const std::unordered_set<core::ClipId>& expandedClipIds
    );
};

} // namespace catchim::editor
