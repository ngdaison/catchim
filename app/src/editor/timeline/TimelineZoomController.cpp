#include "editor/timeline/TimelineZoomController.h"
#include <cmath>

namespace catchim::editor {

TimelineZoomController::TimelineZoomController(double basePixelsPerSecond)
    : m_basePixelsPerSecond(basePixelsPerSecond) {}

void TimelineZoomController::setZoomLevel(double zoom) {
    m_zoomLevel = std::clamp(zoom, m_minZoom, m_maxZoom);
}

void TimelineZoomController::zoomIn(double factor) {
    setZoomLevel(m_zoomLevel * factor);
}

void TimelineZoomController::zoomOut(double factor) {
    setZoomLevel(m_zoomLevel * factor);
}

void TimelineZoomController::zoomFit(core::TimelineTime totalDuration, double viewportWidthPixels) {
    double durationSec = totalDuration.toSeconds();
    if (durationSec <= 0.001 || viewportWidthPixels <= 10.0) {
        setZoomLevel(1.0);
        return;
    }

    // Leave a small 10% margin at end of timeline
    double targetSec = durationSec * 1.1;
    double desiredPixelsPerSec = viewportWidthPixels / targetSec;
    double desiredZoom = desiredPixelsPerSec / m_basePixelsPerSecond;

    setZoomLevel(desiredZoom);
}

double TimelineZoomController::timeToPixel(core::TimelineTime time, double scrollLeft) const {
    double pixelsPerSec = m_basePixelsPerSecond * m_zoomLevel;
    return (time.toSeconds() * pixelsPerSec) - scrollLeft;
}

core::TimelineTime TimelineZoomController::pixelToTime(double pixelX, double scrollLeft) const {
    double pixelsPerSec = m_basePixelsPerSecond * m_zoomLevel;
    if (pixelsPerSec <= 0.0) return core::TimelineTime(0);

    double totalPixels = pixelX + scrollLeft;
    if (totalPixels < 0.0) totalPixels = 0.0;

    double seconds = totalPixels / pixelsPerSec;
    return core::TimelineTime::fromSeconds(seconds);
}

double TimelineZoomController::calculateFitScale(
    double canvasWidth,
    double canvasHeight,
    double containerWidth,
    double containerHeight
) {
    if (canvasWidth <= 0.0 || canvasHeight <= 0.0 || containerWidth <= 0.0 || containerHeight <= 0.0) {
        return 1.0;
    }

    double scaleX = containerWidth / canvasWidth;
    double scaleY = containerHeight / canvasHeight;
    return std::min(scaleX, scaleY);
}

double TimelineZoomController::scaleForPreset(ViewportPreset preset, double fitScale) {
    switch (preset) {
        case ViewportPreset::Fit: return fitScale;
        case ViewportPreset::Scale25: return 0.25;
        case ViewportPreset::Scale50: return 0.50;
        case ViewportPreset::Scale100: return 1.00;
        case ViewportPreset::Scale200: return 2.00;
    }
    return fitScale;
}

} // namespace catchim::editor
