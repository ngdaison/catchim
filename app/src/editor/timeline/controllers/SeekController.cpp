#include "SeekController.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

namespace {
constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
}

core::TimelineTime SeekController::pixelToTime(
    double clientX,
    double containerLeft,
    double scrollLeft,
    double zoomLevel,
    core::TimelineTime duration
) noexcept {
    const double effectiveZoom = zoomLevel > 0.0 ? zoomLevel : 1.0;
    const double mouseX = clientX - containerLeft;

    const double rawTimeSeconds = std::max(
        0.0,
        std::min(
            duration.toSeconds(),
            (mouseX + scrollLeft) / (BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom)
        )
    );

    return core::TimelineTime::fromSeconds(rawTimeSeconds);
}

bool SeekController::isClickGesture(
    double clientX,
    double clientY,
    int64_t timeMs,
    const PendingSeekSession& session
) noexcept {
    const double deltaX = std::abs(clientX - session.downX);
    const double deltaY = std::abs(clientY - session.downY);
    const int64_t deltaTime = timeMs - session.downTimeMs;

    return deltaX <= 5.0 && deltaY <= 5.0 && deltaTime >= 0 && deltaTime <= 500;
}

void SeekController::onMouseDown(
    SeekSource source,
    double clientX,
    double clientY,
    int64_t timeMs
) noexcept {
    pendingSession_ = PendingSeekSession{
        .source = source,
        .downX = clientX,
        .downY = clientY,
        .downTimeMs = timeMs
    };
}

bool SeekController::onClick(
    SeekSource source,
    double clientX,
    double clientY,
    int64_t timeMs,
    double containerLeft,
    double scrollLeft
) {
    if (!pendingSession_.has_value()) {
        return false;
    }

    const auto session = *pendingSession_;
    pendingSession_.reset();

    if (session.source != source) {
        return false;
    }

    if (!isClickGesture(clientX, clientY, timeMs, session)) {
        return false;
    }

    if (config_.isSelecting) {
        return false;
    }

    if (config_.clearSelectedElements) {
        config_.clearSelectedElements();
    }

    const auto rawTime = pixelToTime(
        clientX,
        containerLeft,
        scrollLeft,
        config_.zoomLevel,
        config_.duration
    );

    core::TimelineTime finalTime = rawTime;
    if (config_.activeProjectFps.has_value()) {
        finalTime = core::EuclideanFrameSnapper::snappedSeekTime(
            rawTime,
            config_.duration,
            *config_.activeProjectFps
        );
    }

    if (config_.seek) {
        config_.seek(finalTime);
    }

    if (config_.setTimelineViewState) {
        config_.setTimelineViewState(TimelineViewState{
            .zoomLevel = config_.zoomLevel,
            .scrollLeft = scrollLeft,
            .playheadTime = finalTime
        });
    }

    return true;
}

} // namespace catchim::editor
