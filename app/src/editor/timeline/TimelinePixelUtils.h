#pragma once

#include "core/time/TimelineTime.h"
#include <cmath>

namespace catchim::editor {

/**
 * @brief Utilities for converting timeline time to pixels, snapping to device pixel grid, and cursor centering.
 * Corresponds to web/src/timeline/pixel-utils.ts and web/src/timeline/scale.ts.
 */
class TimelinePixelUtils {
public:
    static constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
    static constexpr double TIMELINE_ZOOM_MIN = 0.1;
    static constexpr double TIMELINE_ZOOM_MAX = 100.0;
    static constexpr double TIMELINE_INDICATOR_LINE_WIDTH_PX = 2.0;

    static double getTimelinePixelsPerSecond(double zoomLevel) noexcept {
        return BASE_TIMELINE_PIXELS_PER_SECOND * zoomLevel;
    }

    static double timelineTimeToPixels(core::TimelineTime time, double zoomLevel) noexcept {
        return time.toSeconds() * getTimelinePixelsPerSecond(zoomLevel);
    }

    static double snapPixelToDeviceGrid(double pixel, double devicePixelRatio = 1.0) noexcept {
        const double safeRatio = (devicePixelRatio > 0.0 && std::isfinite(devicePixelRatio))
            ? devicePixelRatio
            : 1.0;
        return std::round(pixel * safeRatio) / safeRatio;
    }

    static double timelineTimeToSnappedPixels(
        core::TimelineTime time,
        double zoomLevel,
        double devicePixelRatio = 1.0
    ) noexcept {
        const double rawPixel = timelineTimeToPixels(time, zoomLevel);
        return snapPixelToDeviceGrid(rawPixel, devicePixelRatio);
    }

    static double getCenteredLineLeft(
        double centerPixel,
        double lineWidthPx = TIMELINE_INDICATOR_LINE_WIDTH_PX
    ) noexcept {
        return centerPixel - (lineWidthPx / 2.0);
    }
};

} // namespace catchim::editor
