#include "SelectionHitTesting.h"
#include "editor/timeline/TrackLayoutMetrics.h"
#include <algorithm>

namespace catchim::editor {

SelectionRectangle SelectionHitTesting::getNormalizedRectangle(
    const SelectionPoint& startPos,
    const SelectionPoint& endPos
) noexcept {
    return SelectionRectangle{
        .left = std::min(startPos.x, endPos.x),
        .top = std::min(startPos.y, endPos.y),
        .right = std::max(startPos.x, endPos.x),
        .bottom = std::max(startPos.y, endPos.y)
    };
}

bool SelectionHitTesting::isRectangleIntersecting(
    const SelectionRectangle& a,
    const SelectionRectangle& b
) noexcept {
    return !(
        a.right < b.left ||
        a.left > b.right ||
        a.bottom < b.top ||
        a.top > b.bottom
    );
}

std::vector<TimelineElementRef> SelectionHitTesting::resolveTimelineElementIntersections(
    const std::vector<const Track*>& tracks,
    double zoomLevel,
    const SelectionPoint& startPos,
    const SelectionPoint& currentPos,
    double scrollLeft,
    double scrollTop,
    double pixelsPerSecond
) {
    const SelectionPoint adjustedStart{
        startPos.x + scrollLeft,
        startPos.y + scrollTop
    };
    const SelectionPoint adjustedEnd{
        currentPos.x + scrollLeft,
        currentPos.y + scrollTop
    };

    const auto selectionRect = getNormalizedRectangle(adjustedStart, adjustedEnd);
    std::vector<TimelineElementRef> result;

    const double effectiveZoom = zoomLevel > 0.0 ? zoomLevel : 1.0;
    const double pps = pixelsPerSecond > 0.0 ? pixelsPerSecond : 50.0;
    const double pxPerSec = pps * effectiveZoom;

    for (size_t trackIndex = 0; trackIndex < tracks.size(); ++trackIndex) {
        const auto* track = tracks[trackIndex];
        if (!track) continue;

        const double trackTop = TrackLayoutMetrics::getCumulativeHeightBefore(tracks, trackIndex);
        const double trackHeight = TrackLayoutMetrics::getTrackHeight(track->type());
        const double elementTop = TrackLayoutMetrics::TIMELINE_CONTENT_TOP_PADDING + trackTop;
        const double elementBottom = elementTop + trackHeight;

        // Fast vertical culling
        if (elementBottom < selectionRect.top || elementTop > selectionRect.bottom) {
            continue;
        }

        for (const auto& element : track->clips()) {
            const double elementLeft = element.startTime().toSeconds() * pxPerSec;
            if (elementLeft > selectionRect.right) {
                continue;
            }

            const double elementRight = (element.startTime() + element.duration()).toSeconds() * pxPerSec;
            if (elementRight < selectionRect.left) {
                continue;
            }

            result.push_back(TimelineElementRef{
                .trackId = track->id(),
                .elementId = element.id()
            });
        }
    }

    return result;
}

} // namespace catchim::editor
