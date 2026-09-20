#pragma once

#include <cmath>

namespace catchim::editor {

class TimelineInteractionMetrics {
public:
    static constexpr double TIMELINE_DRAG_THRESHOLD_PX = 5.0;
    static constexpr double TIMELINE_HORIZONTAL_WHEEL_STEP_PX = 40.0;
    static constexpr double TIMELINE_ZOOM_BUTTON_FACTOR = 1.7;
    static constexpr double TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD = 0.15;

    static double calculateZoomIn(double currentZoom) noexcept;
    static double calculateZoomOut(double currentZoom) noexcept;

    static bool isDragGesture(
        double startX,
        double startY,
        double currentX,
        double currentY
    ) noexcept;

    static double calculateWheelScrollDelta(double wheelDeltaY) noexcept;
};

} // namespace catchim::editor
