// Feature 220 -- mirrors web/src/timeline/hooks/use-edge-auto-scroll.ts
#include "editor/timeline/TimelineEdgeScrollEngine.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor::timeline {

double computeEdgeScrollDelta(const EdgeScrollParams& p) {
    double scrollSpeed = 0.0;

    if (p.mouseXRelative < p.edgeThreshold && p.scrollLeft > 0.0) {
        double edgeDistance = std::max(0.0, p.mouseXRelative);
        double intensity = 1.0 - edgeDistance / p.edgeThreshold;
        scrollSpeed = -p.maxScrollSpeed * intensity;
    } else if (p.mouseXRelative > p.viewportWidth - p.edgeThreshold
               && p.scrollLeft < p.scrollMax) {
        double edgeDistance = std::max(0.0, p.viewportWidth - p.mouseXRelative);
        double intensity = 1.0 - edgeDistance / p.edgeThreshold;
        scrollSpeed = p.maxScrollSpeed * intensity;
    }

    return scrollSpeed;
}

double applyScrollDelta(double scrollLeft, double delta, double scrollMax) {
    return std::max(0.0, std::min(scrollMax, scrollLeft + delta));
}

} // namespace catchim::editor::timeline
