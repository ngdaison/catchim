#pragma once
// Feature 220 -- mirrors web/src/timeline/hooks/use-edge-auto-scroll.ts

namespace catchim::editor::timeline {

/**
 * Computes the scroll delta (px/frame) for one animation frame during
 * a drag operation, based on the mouse X position relative to the viewport.
 * Returns 0 if not within an edge zone, or if the scroll limit is reached.
 *
 * Mirrors: useEdgeAutoScroll step() inner function logic.
 */
struct EdgeScrollParams {
    double mouseXRelative;  // mouseX - viewportLeft
    double viewportWidth;
    double scrollLeft;
    double scrollMax;
    double edgeThreshold = 100.0;
    double maxScrollSpeed = 15.0;
};

double computeEdgeScrollDelta(const EdgeScrollParams& params);

/**
 * Clamps the new scroll position to [0, scrollMax].
 */
double applyScrollDelta(double scrollLeft, double delta, double scrollMax);

} // namespace catchim::editor::timeline
