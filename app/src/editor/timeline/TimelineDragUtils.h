#pragma once

#include "core/time/TimelineTime.h"
#include "core/time/EuclideanFrameSnapper.h"
#include <algorithm>

namespace catchim::editor {

class TimelineDragUtils {
public:
    static constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;

    static core::TimelineTime getMouseTimeFromClientX(
        double clientX,
        double containerLeft,
        double scrollLeft,
        double zoomLevel
    ) noexcept;

    static core::TimelineTime getMouseTimeSnapped(
        double clientX,
        double containerLeft,
        double scrollLeft,
        double zoomLevel,
        const core::FrameRate& fps
    ) noexcept;

    static core::TimelineTime clampTimeToDuration(
        core::TimelineTime time,
        core::TimelineTime duration
    ) noexcept;
};

} // namespace catchim::editor
