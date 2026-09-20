#include "KeyframeDragController.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

namespace {
constexpr double TIMELINE_DRAG_THRESHOLD_PX = 5.0;
constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
}

KeyframeDragState KeyframeDragController::dragState() const noexcept {
    if (session_.kind != KeyframeDragSessionKind::Active) {
        return KeyframeDragState{
            .isDragging = false,
            .draggingKeyframeIds = {},
            .deltaTicks = 0
        };
    }
    return KeyframeDragState{
        .isDragging = true,
        .draggingKeyframeIds = session_.keyframeIds,
        .deltaTicks = session_.deltaTicks
    };
}

void KeyframeDragController::onKeyframeMouseDown(
    const std::vector<std::string>& keyframes,
    double clientX
) noexcept {
    mouseDownX_ = clientX;
    session_ = KeyframeDragSession{
        .kind = KeyframeDragSessionKind::Pending,
        .keyframeIds = keyframes,
        .startMouseX = clientX,
        .deltaTicks = 0
    };
}

bool KeyframeDragController::onKeyframeClick(
    const std::vector<std::string>& /*keyframes*/,
    double clientX,
    core::TimelineTime indicatorTime
) {
    const bool wasDrag = mouseDownX_.has_value() &&
        std::abs(clientX - *mouseDownX_) > TIMELINE_DRAG_THRESHOLD_PX;
    mouseDownX_.reset();

    if (wasDrag) {
        return false;
    }

    const auto absoluteIndicatorTime = config_.displayedStartTime + indicatorTime;
    core::TimelineTime seekTime = absoluteIndicatorTime;

    if (config_.fps.has_value() && config_.getTotalDuration) {
        seekTime = core::EuclideanFrameSnapper::snappedSeekTime(
            absoluteIndicatorTime,
            config_.getTotalDuration(),
            *config_.fps
        );
    }

    if (config_.seek) {
        config_.seek(seekTime);
    }

    return true;
}

void KeyframeDragController::handleMouseMove(double clientX) noexcept {
    if (session_.kind == KeyframeDragSessionKind::Pending) {
        const double deltaX = std::abs(clientX - session_.startMouseX);
        if (deltaX <= TIMELINE_DRAG_THRESHOLD_PX) {
            return;
        }

        session_.kind = KeyframeDragSessionKind::Active;
        session_.deltaTicks = 0;
    }

    if (session_.kind != KeyframeDragSessionKind::Active) {
        return;
    }

    const double effectiveZoom = config_.zoomLevel > 0.0 ? config_.zoomLevel : 1.0;
    const double pixelsPerSecond = BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom;
    const double rawSeconds = (clientX - session_.startMouseX) / pixelsPerSecond;
    const auto rawDeltaTime = core::TimelineTime::fromSeconds(rawSeconds);

    int64_t finalTicks = rawDeltaTime.ticks();
    if (config_.fps.has_value()) {
        const auto rounded = core::EuclideanFrameSnapper::roundToFrame(rawDeltaTime, *config_.fps);
        if (rounded.has_value()) {
            finalTicks = rounded->ticks();
        }
    }

    session_.deltaTicks = finalTicks;
}

void KeyframeDragController::handleMouseUp() {
    if (session_.kind == KeyframeDragSessionKind::Pending) {
        cancel();
        return;
    }

    if (session_.kind != KeyframeDragSessionKind::Active) {
        return;
    }

    if (!session_.keyframeIds.empty() && session_.deltaTicks != 0 && config_.commitDrag) {
        config_.commitDrag(session_.keyframeIds, session_.deltaTicks);
    }

    cancel();
}

void KeyframeDragController::cancel() noexcept {
    session_ = KeyframeDragSession{
        .kind = KeyframeDragSessionKind::Idle,
        .keyframeIds = {},
        .startMouseX = 0.0,
        .deltaTicks = 0
    };
    mouseDownX_.reset();
}

core::TimelineTime KeyframeDragController::calculateClampedTime(
    core::TimelineTime originalTime,
    int64_t deltaTicks,
    core::TimelineTime maxDuration
) noexcept {
    const int64_t candidateTicks = originalTime.ticks() + deltaTicks;
    const int64_t clampedTicks = std::max(int64_t{0}, std::min(maxDuration.ticks(), candidateTicks));
    return core::TimelineTime(clampedTicks);
}

double KeyframeDragController::getVisualOffsetPx(
    core::TimelineTime indicatorTime,
    double indicatorOffsetPx,
    bool isBeingDragged,
    double elementLeft
) const noexcept {
    if (!isBeingDragged || session_.kind != KeyframeDragSessionKind::Active) {
        return indicatorOffsetPx;
    }

    const auto clampedTime = calculateClampedTime(
        indicatorTime,
        session_.deltaTicks,
        config_.elementDuration
    );

    const double effectiveZoom = config_.zoomLevel > 0.0 ? config_.zoomLevel : 1.0;
    const double snappedX = (config_.displayedStartTime + clampedTime).toSeconds() *
        BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom;

    return snappedX - elementLeft;
}

} // namespace catchim::editor
