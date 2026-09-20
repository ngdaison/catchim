#include "editor/timeline/TimelineCoordinateEngine.h"

namespace catchim::editor {

double TimelineCoordinateEngine::getTimelinePixelsPerSecond(double zoomLevel) noexcept {
    double clampedZoom = std::clamp(zoomLevel, kMinZoomLevel, kMaxZoomLevel);
    return kBasePixelsPerSecond * clampedZoom;
}

double TimelineCoordinateEngine::timelineTimeToPixels(core::TimelineTime time, double zoomLevel) noexcept {
    double seconds = time.toSeconds();
    return seconds * getTimelinePixelsPerSecond(zoomLevel);
}

core::TimelineTime TimelineCoordinateEngine::pixelsToTimelineTime(double pixels, double zoomLevel) noexcept {
    if (pixels <= 0.0) {
        return core::TimelineTime(0);
    }
    double pps = getTimelinePixelsPerSecond(zoomLevel);
    if (pps <= 0.0) {
        return core::TimelineTime(0);
    }
    double seconds = pixels / pps;
    return core::TimelineTime::fromSeconds(seconds);
}

double TimelineCoordinateEngine::snapPixelToDeviceGrid(double pixel, double devicePixelRatio) noexcept {
    double dpr = (devicePixelRatio > 0.0) ? devicePixelRatio : 1.0;
    return std::round(pixel * dpr) / dpr;
}

double TimelineCoordinateEngine::timelineTimeToSnappedPixels(
    core::TimelineTime time,
    double zoomLevel,
    double devicePixelRatio
) noexcept {
    double rawPixels = timelineTimeToPixels(time, zoomLevel);
    return snapPixelToDeviceGrid(rawPixels, devicePixelRatio);
}

double TimelineCoordinateEngine::getCenteredLineLeft(
    double centerPixel,
    double lineWidthPx
) noexcept {
    return centerPixel - (lineWidthPx / 2.0);
}

core::TimelineTime TimelineCoordinateEngine::getMouseTimeFromClientX(
    double clientX,
    double containerLeft,
    double scrollLeft,
    double zoomLevel
) noexcept {
    double mouseX = clientX - containerLeft + scrollLeft;
    if (mouseX <= 0.0) {
        return core::TimelineTime(0);
    }
    return pixelsToTimelineTime(mouseX, zoomLevel);
}

} // namespace catchim::editor
