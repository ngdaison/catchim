#pragma once

#include "editor/timeline/TimelineSnappingEngine.h"
#include "editor/timeline/Track.h"
#include <vector>
#include <unordered_set>
#include <string>

namespace catchim::editor {
class Timeline;
}

namespace catchim::editor::snapping {

/**
 * @brief Factory for extracting timeline snap points from clips, playhead, and animation keyframes.
 * Corresponds to web/src/timeline/element-snap-source.ts, playhead-snap-source.ts, and animation-snap-points.ts.
 */
class TimelineSnapPointSources {
public:
    static std::vector<SnapPoint> getElementEdgeSnapPoints(
        const std::vector<Track>& tracks,
        const std::unordered_set<std::string>& excludeElementIds = {}
    );
    static std::vector<SnapPoint> getElementEdgeSnapPoints(
        const Timeline& timeline,
        const std::unordered_set<std::string>& excludeElementIds = {}
    );

    static std::vector<SnapPoint> getPlayheadSnapPoints(
        core::TimelineTime playheadTime
    );

    static std::vector<SnapPoint> getAnimationKeyframeSnapPoints(
        const std::vector<Track>& tracks,
        const std::unordered_set<std::string>& excludeElementIds = {}
    );
    static std::vector<SnapPoint> getAnimationKeyframeSnapPoints(
        const Timeline& timeline,
        const std::unordered_set<std::string>& excludeElementIds = {}
    );
};

} // namespace catchim::editor::snapping
