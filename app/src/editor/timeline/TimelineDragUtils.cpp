#include "TimelineDragUtils.h"
#include <cmath>

namespace catchim::editor {

core::TimelineTime TimelineDragUtils::getMouseTimeFromClientX(
    double clientX,
    double containerLeft,
    double scrollLeft,
    double zoomLevel
) noexcept {
    const double effectiveZoom = zoomLevel > 0.0 ? zoomLevel : 1.0;
    const double mouseX = clientX - containerLeft + scrollLeft;
    const double seconds = std::max(0.0, mouseX / (BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom));
    return core::TimelineTime::fromSeconds(seconds);
}

core::TimelineTime TimelineDragUtils::getMouseTimeSnapped(
    double clientX,
    double containerLeft,
    double scrollLeft,
    double zoomLevel,
    const core::FrameRate& fps
) noexcept {
    const auto rawTime = getMouseTimeFromClientX(clientX, containerLeft, scrollLeft, zoomLevel);
    const auto rounded = core::EuclideanFrameSnapper::roundToFrame(rawTime, fps);
    return rounded.value_or(rawTime);
}

core::TimelineTime TimelineDragUtils::clampTimeToDuration(
    core::TimelineTime time,
    core::TimelineTime duration
) noexcept {
    return time.clamp(core::TimelineTime(0), duration);
}

} // namespace catchim::editor
