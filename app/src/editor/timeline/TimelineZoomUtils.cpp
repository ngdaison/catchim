#include "editor/timeline/TimelineZoomUtils.h"

namespace catchim::editor {

double TimelineZoomUtils::getTimelineZoomMin(
    core::TimelineTime duration,
    double containerWidth
) noexcept {
    const double safeDurationSeconds = std::max(duration.toSeconds(), 1.0);
    const double safeContainerWidth = containerWidth > 0.0 ? containerWidth : 1000.0;
    const double contentRatioAtMinZoom = 1.0 - PADDING_MAX_RATIO;
    const double availableWidth = safeContainerWidth * contentRatioAtMinZoom;
    const double zoomToFit = availableWidth / (safeDurationSeconds * TimelinePixelUtils::BASE_TIMELINE_PIXELS_PER_SECOND);

    return std::min(TimelinePixelUtils::TIMELINE_ZOOM_MAX, zoomToFit);
}

double TimelineZoomUtils::getZoomPercent(
    double zoomLevel,
    double minZoom,
    double maxZoom
) noexcept {
    if (maxZoom <= minZoom) {
        return 0.0;
    }
    return (zoomLevel - minZoom) / (maxZoom - minZoom);
}

double TimelineZoomUtils::getTimelinePaddingPx(
    double containerWidth,
    double zoomLevel,
    double minZoom
) noexcept {
    const double zoomPercent = getZoomPercent(zoomLevel, minZoom);
    const double paddingTransitionPercent = std::min(zoomPercent / PADDING_MIN_AT_ZOOM_PERCENT, 1.0);
    const double paddingRatio = PADDING_MAX_RATIO -
        (PADDING_MAX_RATIO - PADDING_MIN_RATIO) * paddingTransitionPercent;

    return containerWidth * paddingRatio;
}

double TimelineZoomUtils::sliderToZoom(
    double sliderPosition,
    double minZoom,
    double maxZoom
) noexcept {
    if (minZoom <= 0.0 || maxZoom <= minZoom) {
        return minZoom;
    }
    const double clampedPosition = std::clamp(sliderPosition, 0.0, 1.0);
    return minZoom * std::pow(maxZoom / minZoom, clampedPosition);
}

double TimelineZoomUtils::zoomToSlider(
    double zoomLevel,
    double minZoom,
    double maxZoom
) noexcept {
    if (minZoom <= 0.0 || maxZoom <= minZoom) {
        return 0.0;
    }
    const double clampedZoom = std::clamp(zoomLevel, minZoom, maxZoom);
    return std::log(clampedZoom / minZoom) / std::log(maxZoom / minZoom);
}

} // namespace catchim::editor
