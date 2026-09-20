#pragma once

namespace catchim::editor {

class TimelineToolbarEngine {
public:
    static constexpr double TIMELINE_DRAG_THRESHOLD_PX = 5.0;
    static constexpr double TIMELINE_HORIZONTAL_WHEEL_STEP_PX = 40.0;
    static constexpr double TIMELINE_ZOOM_BUTTON_FACTOR = 1.7;
    static constexpr double TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD = 0.15;

    static double zoomIn(
        double currentZoom,
        double minZoom = 0.01,
        double maxZoom = 100.0,
        double factor = TIMELINE_ZOOM_BUTTON_FACTOR
    ) noexcept;

    static double zoomOut(
        double currentZoom,
        double minZoom = 0.01,
        double maxZoom = 100.0,
        double factor = TIMELINE_ZOOM_BUTTON_FACTOR
    ) noexcept;

    static bool isPlayheadAnchored(
        double playheadPixelX,
        double viewportWidth,
        double threshold = TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD
    ) noexcept;
};

} // namespace catchim::editor
