#include "editor/timeline/TimelineDragData.h"
#include <algorithm>

namespace catchim::editor {

core::TimelineTime TimelineDragEngine::toElementDurationTicks(std::optional<double> seconds) noexcept {
    if (!seconds.has_value()) {
        return DEFAULT_NEW_ELEMENT_DURATION;
    }
    return core::TimelineTime::fromSeconds(*seconds);
}

core::TimelineTime TimelineDragEngine::getMouseTimeFromClientX(
    double clientX,
    double containerLeft,
    double scrollLeft,
    double zoomLevel
) noexcept {
    const double mouseX = clientX - containerLeft + scrollLeft;
    const double pixelsPerSec = TimelinePixelUtils::getTimelinePixelsPerSecond(zoomLevel);
    if (pixelsPerSec <= 0.0) {
        return core::TimelineTime::zero();
    }
    const double seconds = std::max(0.0, mouseX / pixelsPerSec);
    return core::TimelineTime::fromSeconds(seconds);
}

std::string TimelineDragEngine::getDragDataId(const TimelineDragData& data) {
    return std::visit([](const auto& d) -> std::string {
        return d.id;
    }, data);
}

std::string TimelineDragEngine::getDragDataName(const TimelineDragData& data) {
    return std::visit([](const auto& d) -> std::string {
        return d.name;
    }, data);
}

std::string TimelineDragEngine::getDragDataType(const TimelineDragData& data) {
    return std::visit([](const auto& d) -> std::string {
        using T = std::decay_t<decltype(d)>;
        if constexpr (std::is_same_v<T, MediaDragPayload>) return "media";
        if constexpr (std::is_same_v<T, TextDragPayload>) return "text";
        if constexpr (std::is_same_v<T, StickerDragPayload>) return "sticker";
        if constexpr (std::is_same_v<T, GraphicDragPayload>) return "graphic";
        if constexpr (std::is_same_v<T, EffectDragPayload>) return "effect";
        return "unknown";
    }, data);
}

} // namespace catchim::editor
