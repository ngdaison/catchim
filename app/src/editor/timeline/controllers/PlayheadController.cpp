#include "PlayheadController.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

namespace {
constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
constexpr double SNAP_THRESHOLD_PX = 10.0;
}

core::TimelineTime PlayheadController::pixelToTime(
    double clientX,
    double rulerLeft,
    double zoomLevel,
    core::TimelineTime duration
) noexcept {
    const double effectiveZoom = zoomLevel > 0.0 ? zoomLevel : 1.0;
    const double clampedX = std::max(0.0, clientX - rulerLeft);

    const double seconds = std::max(
        0.0,
        std::min(
            duration.toSeconds(),
            clampedX / (BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom)
        )
    );

    return core::TimelineTime::fromSeconds(seconds);
}

void PlayheadController::onPlayheadMouseDown(double clientX, double rulerLeft) {
    session_ = PlayheadScrubSession{
        .didStartFromRuler = false,
        .hasMoved = false,
        .currentTime = std::nullopt
    };

    if (config_.setScrubbing) {
        config_.setScrubbing(true);
    }

    scrub(clientX, rulerLeft, true);
}

void PlayheadController::onRulerMouseDown(double clientX, double rulerLeft) {
    session_ = PlayheadScrubSession{
        .didStartFromRuler = true,
        .hasMoved = false,
        .currentTime = std::nullopt
    };

    if (config_.setScrubbing) {
        config_.setScrubbing(true);
    }

    // No element snapping on initial ruler click to avoid sudden jump
    scrub(clientX, rulerLeft, false);
}

void PlayheadController::handleMouseMove(double clientX, double rulerLeft) {
    if (!session_.has_value()) {
        return;
    }

    scrub(clientX, rulerLeft, true);
    if (session_->didStartFromRuler) {
        session_->hasMoved = true;
    }
}

void PlayheadController::handleMouseUp(double clientX, double rulerLeft, double scrollLeft) {
    if (!session_.has_value()) {
        return;
    }

    const auto currentSession = *session_;
    if (config_.setScrubbing) {
        config_.setScrubbing(false);
    }

    if (currentSession.currentTime.has_value()) {
        if (config_.seek) {
            config_.seek(*currentSession.currentTime);
        }
        if (config_.setTimelineViewState) {
            config_.setTimelineViewState(TimelineViewState{
                .zoomLevel = config_.zoomLevel,
                .scrollLeft = scrollLeft,
                .playheadTime = *currentSession.currentTime
            });
        }
    }

    // Ruler click without dragging: snap to clicked position on mouse up
    if (currentSession.didStartFromRuler && !currentSession.hasMoved) {
        scrub(clientX, rulerLeft, false);
    }

    session_.reset();
}

bool PlayheadController::handlePlaybackUpdate(
    core::TimelineTime time,
    double viewportWidth,
    double contentWidth,
    double& inOutScrollLeft
) {
    if (!config_.isPlaying || session_.has_value()) {
        return false;
    }

    const double effectiveZoom = config_.zoomLevel > 0.0 ? config_.zoomLevel : 1.0;
    const double playheadPixels = time.toSeconds() * BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom;

    const bool isOutOfView = (playheadPixels < inOutScrollLeft) ||
                             (playheadPixels > inOutScrollLeft + viewportWidth);

    if (isOutOfView && viewportWidth > 0.0) {
        const double maxScroll = std::max(0.0, contentWidth - viewportWidth);
        const double desiredScroll = std::max(0.0, std::min(maxScroll, playheadPixels - viewportWidth / 2.0));
        if (std::abs(desiredScroll - inOutScrollLeft) > 0.5) {
            inOutScrollLeft = desiredScroll;
            return true;
        }
    }

    return false;
}

void PlayheadController::scrub(double clientX, double rulerLeft, bool isElementSnappingEnabled) {
    const auto rawTime = pixelToTime(clientX, rulerLeft, config_.zoomLevel, config_.duration);

    core::TimelineTime frameTime = rawTime;
    if (config_.activeProjectFps.has_value()) {
        frameTime = core::EuclideanFrameSnapper::snappedSeekTime(
            rawTime,
            config_.duration,
            *config_.activeProjectFps
        );
    }

    core::TimelineTime finalTime = frameTime;

    if (isElementSnappingEnabled && !config_.isShiftHeld && config_.snapPointsProvider) {
        const double effectiveZoom = config_.zoomLevel > 0.0 ? config_.zoomLevel : 1.0;
        const double maxSnapDistanceSec = SNAP_THRESHOLD_PX / (BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom);
        const auto snapPoints = config_.snapPointsProvider();

        double minDistance = maxSnapDistanceSec;
        for (const auto& sp : snapPoints) {
            const double dist = std::abs((sp - frameTime).toSeconds());
            if (dist < minDistance) {
                minDistance = dist;
                finalTime = sp;
            }
        }
    }

    if (session_.has_value()) {
        session_->currentTime = finalTime;
    }

    if (config_.seek) {
        config_.seek(finalTime);
    }

    lastMouseClientX_ = clientX;
}

} // namespace catchim::editor
