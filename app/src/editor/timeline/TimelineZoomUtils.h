#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/TimelinePixelUtils.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

/**
 * @brief Utilities for timeline zoom levels, exponential slider mapping, and container padding.
 * Corresponds to web/src/timeline/zoom-utils.ts.
 */
class TimelineZoomUtils {
public:
    static constexpr double PADDING_MAX_RATIO = 0.75;
    static constexpr double PADDING_MIN_RATIO = 0.15;
    static constexpr double PADDING_MIN_AT_ZOOM_PERCENT = 0.2;

    static double getTimelineZoomMin(
        core::TimelineTime duration,
        double containerWidth = 1000.0
    ) noexcept;

    static double getZoomPercent(
        double zoomLevel,
        double minZoom,
        double maxZoom = TimelinePixelUtils::TIMELINE_ZOOM_MAX
    ) noexcept;

    static double getTimelinePaddingPx(
        double containerWidth,
        double zoomLevel,
        double minZoom
    ) noexcept;

    static double sliderToZoom(
        double sliderPosition,
        double minZoom,
        double maxZoom = TimelinePixelUtils::TIMELINE_ZOOM_MAX
    ) noexcept;

    static double zoomToSlider(
        double zoomLevel,
        double minZoom,
        double maxZoom = TimelinePixelUtils::TIMELINE_ZOOM_MAX
    ) noexcept;
};

} // namespace catchim::editor
