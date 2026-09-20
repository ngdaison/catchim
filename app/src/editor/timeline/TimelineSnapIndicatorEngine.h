#pragma once
// Feature 221 -- mirrors web/src/timeline/hooks/use-snap-indicator-position.ts

namespace catchim::editor::timeline {

// TIMELINE_TRACK_LABELS_COLUMN_WIDTH_PX = 140 (layout constant)
inline constexpr double TIMELINE_TRACK_LABELS_COLUMN_WIDTH_PX = 140.0;

struct SnapIndicatorPosition {
    double leftPosition;
    double topPosition;
    double height;
};

struct SnapIndicatorParams {
    long long snapTimeTicks;   // snapPoint.time (0 if no snap point)
    bool hasSnapPoint;
    double zoomLevel;
    double scrollLeft;
    double containerHeight;    // timeline container height, 0 -> 400 fallback
    double edgePaddingPx = 8.0;
};

/**
 * Computes the left/top/height for the snap indicator overlay.
 * Mirrors: useSnapIndicatorPosition return value.
 */
SnapIndicatorPosition computeSnapIndicatorPosition(const SnapIndicatorParams& params);

} // namespace catchim::editor::timeline
