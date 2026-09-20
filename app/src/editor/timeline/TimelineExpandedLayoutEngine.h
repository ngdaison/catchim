#pragma once

#include "editor/timeline/Track.h"
#include "editor/timeline/TimelineLayoutEngine.h"
#include <string>
#include <vector>
#include <unordered_set>

namespace catchim::editor {

class TimelineExpandedLayoutEngine {
public:
    static constexpr double KEYFRAME_LANE_HEIGHT_PX = 24.0;

    static std::string getPropertyLabel(const std::string& path);

    static std::vector<ExpandedRow> getExpandedRowsFromPaths(
        const std::vector<std::string>& propertyPaths
    );

    static std::vector<ExpandedRow> getExpandedRowsForClip(
        const Clip& clip
    );

    static double getExpansionHeight(
        const std::vector<ExpandedRow>& rows
    ) noexcept;

    static double computeTrackExpansionHeight(
        const Track& track,
        const std::unordered_set<std::string>& expandedElementIds
    );

    static std::vector<ExpandedRow> getTrackExpandedRows(
        const Track& track,
        const std::unordered_set<std::string>& expandedElementIds
    );
};

} // namespace catchim::editor
