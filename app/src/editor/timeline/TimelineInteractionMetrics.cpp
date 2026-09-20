#include "TimelineInteractionMetrics.h"
#include <algorithm>

namespace catchim::editor {

double TimelineInteractionMetrics::calculateZoomIn(double currentZoom) noexcept {
    return std::min(100.0, currentZoom * TIMELINE_ZOOM_BUTTON_FACTOR);
}

double TimelineInteractionMetrics::calculateZoomOut(double currentZoom) noexcept {
    return std::max(0.1, currentZoom / TIMELINE_ZOOM_BUTTON_FACTOR);
}

bool TimelineInteractionMetrics::isDragGesture(
    double startX,
    double startY,
    double currentX,
    double currentY
) noexcept {
    const double dx = currentX - startX;
    const double dy = currentY - startY;
    return std::sqrt(dx * dx + dy * dy) > TIMELINE_DRAG_THRESHOLD_PX;
}

double TimelineInteractionMetrics::calculateWheelScrollDelta(double wheelDeltaY) noexcept {
    return (wheelDeltaY > 0.0 ? 1.0 : (wheelDeltaY < 0.0 ? -1.0 : 0.0)) * TIMELINE_HORIZONTAL_WHEEL_STEP_PX;
}

} // namespace catchim::editor
