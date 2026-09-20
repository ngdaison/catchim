#include "TimelineInteractiveZoomController.h"
#include "editor/timeline/TimelineZoomUtils.h"
#include "editor/timeline/TimelinePixelUtils.h"
#include <cmath>

namespace catchim::editor {

TimelineInteractiveZoomController::TimelineInteractiveZoomController(
    InteractiveZoomConfig config,
    double initialZoom
)
    : config_(std::move(config)),
      zoomLevel_(clampZoom(initialZoom, config_.minZoom, config_.maxZoom)),
      previousZoom_(zoomLevel_),
      hasInitialized_(true) {}

double TimelineInteractiveZoomController::clampZoom(
    double zoomLevel,
    double minZoom,
    double maxZoom
) noexcept {
    return std::clamp(zoomLevel, minZoom, maxZoom);
}

void TimelineInteractiveZoomController::setZoomLevel(
    double newZoom,
    double scrollWidth,
    double clientWidth
) {
    const double clamped = clampZoom(newZoom, config_.minZoom, config_.maxZoom);
    if (std::abs(clamped - zoomLevel_) < 1e-6) {
        return;
    }

    preZoomScrollLeft_ = scrollLeft_;
    zoomLevel_ = clamped;
    applyZoomLayout(zoomLevel_, scrollWidth, clientWidth);
}

void TimelineInteractiveZoomController::updateZoom(
    std::function<double(double)> updater,
    double scrollWidth,
    double clientWidth
) {
    if (updater) {
        setZoomLevel(updater(zoomLevel_), scrollWidth, clientWidth);
    }
}

bool TimelineInteractiveZoomController::handleWheel(
    double deltaX,
    double deltaY,
    bool isCtrlOrMeta,
    bool isShift,
    double scrollWidth,
    double clientWidth
) {
    const bool isHorizontalScrollGesture = isShift || std::abs(deltaX) > std::abs(deltaY);
    if (isHorizontalScrollGesture) {
        return false;
    }

    if (isCtrlOrMeta) {
        const double cappedDelta = std::clamp(deltaY, -30.0, 30.0);
        const double zoomFactor = std::exp(-cappedDelta / 300.0);
        setZoomLevel(zoomLevel_ * zoomFactor, scrollWidth, clientWidth);
        return true;
    }

    return false;
}

void TimelineInteractiveZoomController::reconcileInitialAndMinZoom(
    double minZoom,
    std::optional<double> initialZoom
) {
    config_.minZoom = minZoom;
    if (initialZoom.has_value() && !hasInitialized_) {
        hasInitialized_ = true;
        setZoomLevel(clampZoom(*initialZoom, minZoom, config_.maxZoom));
        return;
    }

    if (zoomLevel_ < minZoom) {
        setZoomLevel(minZoom);
    }
}

void TimelineInteractiveZoomController::applyZoomLayout(
    double newZoom,
    double scrollWidth,
    double clientWidth
) {
    if (std::abs(previousZoom_ - newZoom) < 1e-6) {
        return;
    }

    const double currentScrollLeft = preZoomScrollLeft_;
    core::TimelineTime playheadTime(0);
    if (config_.getCurrentPlayheadTime) {
        playheadTime = config_.getCurrentPlayheadTime();
    }

    const double sliderPercent = TimelineZoomUtils::zoomToSlider(newZoom, config_.minZoom, config_.maxZoom);
    const double previousSliderPercent = TimelineZoomUtils::zoomToSlider(previousZoom_, config_.minZoom, config_.maxZoom);

    const bool isCrossingThresholdUp =
        previousSliderPercent < TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD &&
        sliderPercent >= TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD;
    const bool isCrossingThresholdDown =
        previousSliderPercent >= TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD &&
        sliderPercent < TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD;

    auto clampScroll = [&](double val) -> double {
        const double maxScroll = std::max(0.0, scrollWidth - clientWidth);
        if (maxScroll <= 0.0) return std::max(0.0, val);
        return std::clamp(val, 0.0, maxScroll);
    };

    if (isCrossingThresholdUp) {
        prePlayheadAnchorScrollLeft_ = currentScrollLeft;
        isInPlayheadAnchorMode_ = true;
    }

    if (sliderPercent >= TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD) {
        const double playheadPixelsBefore = TimelinePixelUtils::timelineTimeToPixels(playheadTime, previousZoom_);
        const double playheadPixelsAfter = TimelinePixelUtils::timelineTimeToPixels(playheadTime, newZoom);
        const double viewportOffset = playheadPixelsBefore - currentScrollLeft;
        const double nextScrollLeft = playheadPixelsAfter - viewportOffset;
        scrollLeft_ = clampScroll(nextScrollLeft);
    } else if (isCrossingThresholdDown && isInPlayheadAnchorMode_) {
        scrollLeft_ = clampScroll(prePlayheadAnchorScrollLeft_);
        isInPlayheadAnchorMode_ = false;
    }

    previousZoom_ = newZoom;

    if (config_.onViewStateChanged) {
        TimelineViewState vs;
        vs.zoomLevel = newZoom;
        vs.scrollLeft = scrollLeft_;
        vs.playheadTime = playheadTime;
        config_.onViewStateChanged(vs);
    }
}

} // namespace catchim::editor
