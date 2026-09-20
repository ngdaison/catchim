#include "TimelineElementInteractionController.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

namespace {
constexpr double TIMELINE_DRAG_THRESHOLD_PX = 5.0;
constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
}

void TimelineElementInteractionController::onMouseDown(
    core::TrackId trackId,
    core::ClipId elementId,
    core::TimelineTime startTime,
    double clientX,
    double clientY,
    const std::vector<core::ClipId>& selectedIds
) {
    isDragging_ = false;
    currentDeltaTime_ = core::TimelineTime(0);

    std::vector<core::ClipId> targets = selectedIds;
    if (std::find(targets.begin(), targets.end(), elementId) == targets.end()) {
        targets.push_back(elementId);
    }

    snapshot_ = MousedownSnapshot{
        .originX = clientX,
        .originY = clientY,
        .elementId = elementId,
        .trackId = trackId,
        .startElementTime = startTime,
        .clickOffsetTime = core::TimelineTime(0),
        .selectedElementIds = std::move(targets)
    };
}

void TimelineElementInteractionController::handleMouseMove(double clientX, double clientY) {
    if (!snapshot_.has_value()) {
        return;
    }

    const double deltaX = clientX - snapshot_->originX;
    const double deltaY = clientY - snapshot_->originY;
    const double dist = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    if (!isDragging_) {
        if (dist > TIMELINE_DRAG_THRESHOLD_PX) {
            isDragging_ = true;
        } else {
            return;
        }
    }

    const double effectiveZoom = config_.zoomLevel > 0.0 ? config_.zoomLevel : 1.0;
    const double deltaSeconds = deltaX / (BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom);
    currentDeltaTime_ = core::TimelineTime::fromSeconds(deltaSeconds);

    if (config_.onPreviewMoves) {
        config_.onPreviewMoves(snapshot_->selectedElementIds, currentDeltaTime_);
    }
}

void TimelineElementInteractionController::handleMouseUp(
    double /*clientX*/,
    double /*clientY*/,
    bool isMultiKey
) {
    if (!snapshot_.has_value()) {
        return;
    }

    const auto snap = *snapshot_;
    const bool wasDrag = isDragging_;
    const auto deltaToCommit = currentDeltaTime_;

    cancel();

    if (wasDrag) {
        if (config_.onCommitMoves && deltaToCommit != core::TimelineTime(0)) {
            config_.onCommitMoves(snap.selectedElementIds, deltaToCommit);
        }
    } else {
        if (config_.onElementClick) {
            config_.onElementClick(snap.elementId, isMultiKey);
        }
    }
}

void TimelineElementInteractionController::cancel() noexcept {
    snapshot_.reset();
    isDragging_ = false;
    currentDeltaTime_ = core::TimelineTime(0);
}

} // namespace catchim::editor
