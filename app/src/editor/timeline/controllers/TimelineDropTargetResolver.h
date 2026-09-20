#pragma once

#include "editor/timeline/Track.h"
#include "editor/timeline/Clip.h"
#include "editor/timeline/Timeline.h"
#include "core/time/TimelineTime.h"
#include <optional>
#include <string>
#include <vector>

namespace catchim::editor {

struct DropTargetElement {
    core::ClipId elementId;
    core::TrackId trackId;

    bool operator==(const DropTargetElement& other) const = default;
};

struct DropTarget {
    size_t trackIndex{0};
    bool isNewTrack{false};
    std::optional<std::string> insertPosition{std::nullopt}; // "above", "below", or nullopt
    core::TimelineTime xPosition{0};
    std::optional<DropTargetElement> targetElement{std::nullopt};

    bool operator==(const DropTarget& other) const = default;
};

struct TrackAtYResult {
    size_t trackIndex{0};
    double relativeY{0.0};
};

struct ComputeDropTargetParams {
    ClipType clipType{ClipType::Video};
    double mouseX{0.0};
    double mouseY{0.0};
    core::TimelineTime playheadTime{0};
    bool isExternalDrop{false};
    core::TimelineTime elementDuration{core::TimelineTime::fromSeconds(5.0)};
    double pixelsPerSecond{50.0};
    double zoomLevel{1.0};
    std::optional<std::string> verticalDragDirection{std::nullopt}; // "up" or "down"
    std::optional<core::TimelineTime> startTimeOverride{std::nullopt};
    std::vector<ClipType> targetElementTypes;
};

class TimelineDropTargetResolver {
public:
    static std::optional<DropTargetElement> findElementAtPosition(
        double mouseX,
        const Timeline& timeline,
        size_t trackIndex,
        const std::vector<ClipType>& targetElementTypes,
        double pixelsPerSecond,
        double zoomLevel
    ) noexcept;

    static std::optional<TrackAtYResult> getTrackAtY(
        double mouseY,
        const Timeline& timeline,
        const std::optional<std::string>& verticalDragDirection = std::nullopt
    ) noexcept;

    static DropTarget computeDropTarget(
        const Timeline& timeline,
        const ComputeDropTargetParams& params
    );

    static double getDropLineY(
        const DropTarget& dropTarget,
        const Timeline& timeline
    ) noexcept;
};

} // namespace catchim::editor
