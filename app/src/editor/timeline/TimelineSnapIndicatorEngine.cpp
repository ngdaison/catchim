// Feature 221 -- mirrors web/src/timeline/hooks/use-snap-indicator-position.ts
#include "editor/timeline/TimelineSnapIndicatorEngine.h"
#include "editor/timeline/TimelinePixelUtils.h"
#include <algorithm>

namespace catchim::editor::timeline {

SnapIndicatorPosition computeSnapIndicatorPosition(const SnapIndicatorParams& p) {
    double timelineContainerHeight = (p.containerHeight > 0.0) ? p.containerHeight : 400.0;
    double totalHeight = timelineContainerHeight - p.edgePaddingPx;

    long long snapTime = p.hasSnapPoint ? p.snapTimeTicks : 0LL;
    double timelinePosition = TimelinePixelUtils::timelineTimeToSnappedPixels(
        core::TimelineTime::fromTicks(snapTime), p.zoomLevel);
    double leftPosition = TIMELINE_TRACK_LABELS_COLUMN_WIDTH_PX + timelinePosition - p.scrollLeft;

    return SnapIndicatorPosition{
        .leftPosition = leftPosition,
        .topPosition  = 0.0,
        .height       = totalHeight
    };
}

} // namespace catchim::editor::timeline
