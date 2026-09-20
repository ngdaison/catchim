#pragma once
// Feature 226 -- mirrors web/src/timeline/bookmarks/hooks/use-bookmark-drag.ts
#include "core/time/TimelineTime.h"
#include <optional>
#include <vector>

namespace catchim::editor::timeline {

inline constexpr double TIMELINE_BOOKMARK_DRAG_THRESHOLD_PX = 3.0;

struct BookmarkDragState {
    bool isDragging{false};
    core::TimelineTime originalTime{0};
    core::TimelineTime currentTime{0};
    std::optional<core::TimelineTime> snappedPoint;
};

class BookmarkDragController {
public:
    BookmarkDragController() = default;

    void startPendingDrag(core::TimelineTime bookmarkTime, double mouseX, double mouseY);

    // Updates drag position, applies drag threshold and optional snapping
    void updateDrag(
        double mouseX,
        double mouseY,
        double zoomLevel,
        bool isShiftHeld,
        const std::vector<core::TimelineTime>& snapCandidates = {},
        double snapThresholdPx = 10.0
    );

    std::optional<core::TimelineTime> endDrag();
    void cancelDrag();

    const BookmarkDragState& state() const noexcept { return state_; }
    bool isDragging() const noexcept { return state_.isDragging; }

private:
    BookmarkDragState state_;
    bool isPending_{false};
    double startMouseX_{0.0};
    double startMouseY_{0.0};
};

} // namespace catchim::editor::timeline
