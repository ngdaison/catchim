#pragma once

#include "core/ids/Ids.h"
#include "core/math/Bezier.h"
#include "core/time/TimelineTime.h"
#include "editor/animation/AnimationChannel.h"
#include "editor/clipboard/ClipboardKeyframeEngine.h"
#include <string>
#include <vector>
#include <optional>

namespace catchim::editor {

class Timeline;

enum class GraphEditorUnavailableReason {
    None,
    NoKeyframeSelected,
    MultipleKeyframesSelected,
    SelectedKeyframesSpanMultipleElements,
    SelectedKeyframesAreNotAdjacent,
    SelectedPropertiesHaveNoSharedComponent,
    SelectedElementMissing,
    SelectedElementHasNoAnimations,
    SelectedKeyframeHasNoScalarChannel,
    SelectedKeyframeMissingOnChannel,
    SelectedKeyframeHasNoNextSegment,
    SelectedSegmentIsHold,
    SelectedSegmentIsFlat
};

struct GraphEditorComponentOption {
    std::string key;
    std::string label;
};

struct GraphEditorResolvedSegment {
    std::string propertyPath;
    core::TimelineTime keyframeTime{0};
    core::CubicBezier cubicBezier;
    double referenceSpanValue{1.0};
};

struct GraphEditorSelectionState {
    bool isReady{false};
    GraphEditorUnavailableReason reason{GraphEditorUnavailableReason::None};
    std::string message;
    std::vector<GraphEditorComponentOption> componentOptions;
    std::string activeComponentKey;

    std::optional<core::TrackId> trackId{std::nullopt};
    std::optional<core::ClipId> elementId{std::nullopt};
    std::vector<GraphEditorResolvedSegment> segments;
    core::CubicBezier cubicBezier{core::CubicBezier::ease()};
};

class GraphEditorSessionEngine {
public:
    static constexpr double FLAT_VALUE_EPSILON = 1e-6;

    static GraphEditorSelectionState resolveSelectionState(
        const Timeline& timeline,
        const std::vector<SelectedKeyframeRef>& selectedKeyframes,
        const std::string& activeComponentKey = ""
    );

    static double getReferenceSpanValue(
        const AnimationChannel& channel,
        size_t keyframeIndex
    ) noexcept;

    static GraphEditorSelectionState createUnavailableState(
        GraphEditorUnavailableReason reason,
        std::string message,
        std::vector<GraphEditorComponentOption> componentOptions = {},
        std::string activeComponentKey = ""
    );
};

} // namespace catchim::editor
