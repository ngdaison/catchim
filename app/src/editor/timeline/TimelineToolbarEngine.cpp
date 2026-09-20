#include "TimelineToolbarEngine.h"
#include <algorithm>

namespace catchim::editor {

double TimelineToolbarEngine::zoomIn(
    double currentZoom,
    double minZoom,
    double maxZoom,
    double factor
) noexcept {
    double nextZoom = currentZoom * factor;
    return std::clamp(nextZoom, minZoom, maxZoom);
}

double TimelineToolbarEngine::zoomOut(
    double currentZoom,
    double minZoom,
    double maxZoom,
    double factor
) noexcept {
    if (factor <= 0.0) return currentZoom;
    double nextZoom = currentZoom / factor;
    return std::clamp(nextZoom, minZoom, maxZoom);
}

bool TimelineToolbarEngine::isPlayheadAnchored(
    double playheadPixelX,
    double viewportWidth,
    double threshold
) noexcept {
    if (viewportWidth <= 0.0) {
        return false;
    }
    double fraction = playheadPixelX / viewportWidth;
    return fraction >= threshold && fraction <= (1.0 - threshold);
}

} // namespace catchim::editor
