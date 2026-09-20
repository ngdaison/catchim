#pragma once

#include "core/time/TimelineTime.h"
#include <algorithm>

namespace catchim::editor {

enum class ViewportPreset {
    Fit,
    Scale25,
    Scale50,
    Scale100,
    Scale200
};

class TimelineZoomController {
public:
    TimelineZoomController(double basePixelsPerSecond = 60.0);

    double zoomLevel() const noexcept { return m_zoomLevel; }
    void setZoomLevel(double zoom);

    void zoomIn(double factor = 1.25);
    void zoomOut(double factor = 0.8);
    void zoomFit(core::TimelineTime totalDuration, double viewportWidthPixels);

    // Coordinate transformations
    double timeToPixel(core::TimelineTime time, double scrollLeft = 0.0) const;
    core::TimelineTime pixelToTime(double pixelX, double scrollLeft = 0.0) const;

    // Viewport fit calculations
    static double calculateFitScale(
        double canvasWidth,
        double canvasHeight,
        double containerWidth,
        double containerHeight
    );

    static double scaleForPreset(ViewportPreset preset, double fitScale);

private:
    double m_basePixelsPerSecond{60.0};
    double m_zoomLevel{1.0};
    double m_minZoom{0.05};
    double m_maxZoom{20.0};
};

} // namespace catchim::editor
