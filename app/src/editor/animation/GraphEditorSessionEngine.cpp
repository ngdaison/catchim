#include "GraphEditorSessionEngine.h"
#include "editor/timeline/Timeline.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

GraphEditorSelectionState GraphEditorSessionEngine::createUnavailableState(
    GraphEditorUnavailableReason reason,
    std::string message,
    std::vector<GraphEditorComponentOption> componentOptions,
    std::string activeComponentKey
) {
    return GraphEditorSelectionState{
        .isReady = false,
        .reason = reason,
        .message = std::move(message),
        .componentOptions = std::move(componentOptions),
        .activeComponentKey = std::move(activeComponentKey),
        .trackId = std::nullopt,
        .elementId = std::nullopt,
        .segments = {},
        .cubicBezier = core::CubicBezier::ease()
    };
}

double GraphEditorSessionEngine::getReferenceSpanValue(
    const AnimationChannel& channel,
    size_t keyframeIndex
) noexcept {
    const auto& keys = channel.keyframes();
    if (keys.empty() || keyframeIndex >= keys.size()) {
        return 1.0;
    }

    // Search left
    if (keyframeIndex > 0) {
        for (size_t i = keyframeIndex; i > 0; --i) {
            const double span = std::abs(keys[i].value - keys[i - 1].value);
            if (span > FLAT_VALUE_EPSILON) {
                return span;
            }
        }
    }

    // Search right
    if (keyframeIndex + 1 < keys.size()) {
        for (size_t i = keyframeIndex + 1; i + 1 < keys.size(); ++i) {
            const double span = std::abs(keys[i + 1].value - keys[i].value);
            if (span > FLAT_VALUE_EPSILON) {
                return span;
            }
        }
    }

    return 1.0;
}

GraphEditorSelectionState GraphEditorSessionEngine::resolveSelectionState(
    const Timeline& timeline,
    const std::vector<SelectedKeyframeRef>& selectedKeyframes,
    const std::string& activeComponentKey
) {
    if (selectedKeyframes.empty()) {
        return createUnavailableState(
            GraphEditorUnavailableReason::NoKeyframeSelected,
            "Select a keyframe to edit its curve."
        );
    }

    // Check if multiple keyframes across elements or properties are selected
    const auto& first = selectedKeyframes.front();
    for (const auto& kf : selectedKeyframes) {
        if (kf.clipId != first.clipId) {
            return createUnavailableState(
                GraphEditorUnavailableReason::SelectedKeyframesSpanMultipleElements,
                "Keyframes must belong to the same element."
            );
        }
    }

    const Clip* element = timeline.findClip(first.clipId);
    if (!element) {
        return createUnavailableState(
            GraphEditorUnavailableReason::SelectedElementMissing,
            "Selected element could not be found."
        );
    }

    if (element->animationChannels().empty()) {
        return createUnavailableState(
            GraphEditorUnavailableReason::SelectedElementHasNoAnimations,
            "Element has no animated properties."
        );
    }

    // Group by property
    const std::string propPath = first.propertyPath;
    const AnimationChannel* channel = element->findAnimationChannel(propPath);
    if (!channel) {
        return createUnavailableState(
            GraphEditorUnavailableReason::SelectedKeyframeHasNoScalarChannel,
            "No animation channel found for property."
        );
    }

    // Find keyframe index
    const auto& keys = channel->keyframes();
    auto it = std::find_if(keys.begin(), keys.end(), [&](const Keyframe& k) {
        return k.time == first.keyframeTime;
    });

    if (it == keys.end()) {
        return createUnavailableState(
            GraphEditorUnavailableReason::SelectedKeyframeMissingOnChannel,
            "Keyframe was not found on the channel."
        );
    }

    size_t kfIndex = static_cast<size_t>(std::distance(keys.begin(), it));
    if (kfIndex + 1 >= keys.size()) {
        return createUnavailableState(
            GraphEditorUnavailableReason::SelectedKeyframeHasNoNextSegment,
            "Last keyframe on a channel has no subsequent curve segment."
        );
    }

    const auto& currentKey = keys[kfIndex];
    if (currentKey.interpolation == KeyframeInterpolation::Hold) {
        return createUnavailableState(
            GraphEditorUnavailableReason::SelectedSegmentIsHold,
            "Hold segments do not support curve easing."
        );
    }

    // Build component options
    std::vector<GraphEditorComponentOption> compOptions;
    compOptions.push_back(GraphEditorComponentOption{.key = "value", .label = "Value"});

    core::CubicBezier curve{
        currentKey.bezierX1,
        currentKey.bezierY1,
        currentKey.bezierX2,
        currentKey.bezierY2
    };

    double refSpan = getReferenceSpanValue(*channel, kfIndex);

    GraphEditorResolvedSegment segment{
        .propertyPath = propPath,
        .keyframeTime = currentKey.time,
        .cubicBezier = curve,
        .referenceSpanValue = refSpan
    };

    GraphEditorSelectionState state;
    state.isReady = true;
    state.reason = GraphEditorUnavailableReason::None;
    state.message = "Ready";
    state.componentOptions = std::move(compOptions);
    state.activeComponentKey = activeComponentKey.empty() ? "value" : activeComponentKey;
    state.trackId = first.trackId;
    state.elementId = first.clipId;
    state.segments.push_back(segment);
    state.cubicBezier = curve;

    return state;
}

} // namespace catchim::editor
