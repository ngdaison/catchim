#pragma once

#include "core/ids/Ids.h"
#include "editor/timeline/Track.h"
#include <vector>

namespace catchim::editor {

struct SelectionPoint {
    double x{0.0};
    double y{0.0};

    bool operator==(const SelectionPoint& other) const = default;
};

struct SelectionRectangle {
    double left{0.0};
    double top{0.0};
    double right{0.0};
    double bottom{0.0};

    bool operator==(const SelectionRectangle& other) const = default;
};

struct TimelineElementRef {
    core::TrackId trackId{core::TrackId::empty()};
    core::ClipId elementId{core::ClipId::empty()};

    bool operator==(const TimelineElementRef& other) const = default;
};

class SelectionHitTesting {
public:
    static SelectionRectangle getNormalizedRectangle(
        const SelectionPoint& startPos,
        const SelectionPoint& endPos
    ) noexcept;

    static bool isRectangleIntersecting(
        const SelectionRectangle& a,
        const SelectionRectangle& b
    ) noexcept;

    static std::vector<TimelineElementRef> resolveTimelineElementIntersections(
        const std::vector<const Track*>& tracks,
        double zoomLevel,
        const SelectionPoint& startPos,
        const SelectionPoint& currentPos,
        double scrollLeft = 0.0,
        double scrollTop = 0.0,
        double pixelsPerSecond = 50.0
    );
};

} // namespace catchim::editor
