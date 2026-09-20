// Feature 226 -- mirrors web/src/timeline/bookmarks/hooks/use-bookmark-drag.ts
#include "editor/timeline/controllers/BookmarkDragController.h"
#include <cmath>
#include <limits>

namespace {
    constexpr double BASE_PPS = 50.0;
}

namespace catchim::editor::timeline {

void BookmarkDragController::startPendingDrag(core::TimelineTime bookmarkTime, double mouseX, double mouseY) {
    state_.isDragging = false;
    state_.originalTime = bookmarkTime;
    state_.currentTime = bookmarkTime;
    state_.snappedPoint = std::nullopt;

    isPending_ = true;
    startMouseX_ = mouseX;
    startMouseY_ = mouseY;
}

void BookmarkDragController::updateDrag(
    double mouseX,
    double mouseY,
    double zoomLevel,
    bool isShiftHeld,
    const std::vector<core::TimelineTime>& snapCandidates,
    double snapThresholdPx
) {
    if (!isPending_ && !state_.isDragging) {
        return;
    }

    double dx = mouseX - startMouseX_;
    double dy = mouseY - startMouseY_;
    double dist = std::hypot(dx, dy);

    if (!state_.isDragging) {
        if (dist >= TIMELINE_BOOKMARK_DRAG_THRESHOLD_PX) {
            state_.isDragging = true;
        } else {
            return;
        }
    }

    double pps = BASE_PPS * std::max(zoomLevel, 0.01);
    double deltaSeconds = dx / pps;
    double newSeconds = std::max(0.0, state_.originalTime.toSeconds() + deltaSeconds);
    core::TimelineTime newTime = core::TimelineTime::fromSeconds(newSeconds);

    state_.snappedPoint = std::nullopt;

    if (!isShiftHeld && !snapCandidates.empty()) {
        double snapThresholdSec = snapThresholdPx / pps;
        double bestDist = snapThresholdSec;
        std::optional<core::TimelineTime> bestCandidate;

        for (const auto& candidate : snapCandidates) {
            double cSec = candidate.toSeconds();
            double diff = std::abs(cSec - newSeconds);
            if (diff <= bestDist) {
                bestDist = diff;
                bestCandidate = candidate;
            }
        }

        if (bestCandidate.has_value()) {
            state_.snappedPoint = bestCandidate;
            newTime = *bestCandidate;
        }
    }

    state_.currentTime = newTime;
}

std::optional<core::TimelineTime> BookmarkDragController::endDrag() {
    if (!state_.isDragging) {
        cancelDrag();
        return std::nullopt;
    }
    core::TimelineTime finalTime = state_.currentTime;
    cancelDrag();
    return finalTime;
}

void BookmarkDragController::cancelDrag() {
    state_.isDragging = false;
    state_.snappedPoint = std::nullopt;
    isPending_ = false;
}

} // namespace catchim::editor::timeline
