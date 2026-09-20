#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/TimelinePixelUtils.h"
#include "editor/timeline/TimelineDragSource.h"
#include <string>
#include <vector>
#include <optional>

namespace catchim::editor {

using MediaDragData = MediaDragPayload;
using TextDragData = TextDragPayload;
using StickerDragData = StickerDragPayload;
using GraphicDragData = GraphicDragPayload;
using EffectDragData = EffectDragPayload;

/**
 * @brief Utilities for timeline drag & drop data and element creation defaults.
 * Corresponds to web/src/timeline/drag.ts, drag-utils.ts, and creation.ts.
 */
class TimelineDragEngine {
public:
    static inline const core::TimelineTime DEFAULT_NEW_ELEMENT_DURATION = core::TimelineTime::fromSeconds(5.0);

    static core::TimelineTime toElementDurationTicks(std::optional<double> seconds) noexcept;

    static core::TimelineTime getMouseTimeFromClientX(
        double clientX,
        double containerLeft,
        double scrollLeft,
        double zoomLevel
    ) noexcept;

    static std::string getDragDataId(const TimelineDragData& data);
    static std::string getDragDataName(const TimelineDragData& data);
    static std::string getDragDataType(const TimelineDragData& data);
};

} // namespace catchim::editor
