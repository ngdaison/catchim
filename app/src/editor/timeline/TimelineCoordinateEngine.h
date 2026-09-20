#pragma once

#include "core/time/TimelineTime.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

class TimelineCoordinateEngine {
public:
    static constexpr double kBasePixelsPerSecond = 50.0;
    static constexpr double kMinZoomLevel = 0.1;
    static constexpr double kMaxZoomLevel = 100.0;
    static constexpr double kIndicatorLineWidthPx = 2.0;

    // Returns the number of timeline horizontal pixels representing one second at the given zoom level
    static double getTimelinePixelsPerSecond(double zoomLevel) noexcept;

    // Converts timeline time to continuous horizontal pixels
    static double timelineTimeToPixels(core::TimelineTime time, double zoomLevel) noexcept;

    // Converts horizontal pixels back to timeline time
    static core::TimelineTime pixelsToTimelineTime(double pixels, double zoomLevel) noexcept;

    // Snaps a floating-point pixel coordinate to the physical device pixel grid (HiDPI support)
    static double snapPixelToDeviceGrid(double pixel, double devicePixelRatio = 1.0) noexcept;

    // Converts timeline time directly to snapped pixel coordinates
    static double timelineTimeToSnappedPixels(
        core::TimelineTime time,
        double zoomLevel,
        double devicePixelRatio = 1.0
    ) noexcept;

    // Calculates the left coordinate for a centered indicator line (e.g. playhead line)
    static double getCenteredLineLeft(
        double centerPixel,
        double lineWidthPx = kIndicatorLineWidthPx
    ) noexcept;

    // Converts client/mouse X coordinate inside a scrollable container to timeline time
    static core::TimelineTime getMouseTimeFromClientX(
        double clientX,
        double containerLeft,
        double scrollLeft,
        double zoomLevel
    ) noexcept;
};

} // namespace catchim::editor
