#pragma once

#include "core/time/TimelineTime.h"
#include "editor/animation/AnimationChannel.h"
#include "editor/animation/Keyframe.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace catchim::editor {

struct GroupKeyframeRef {
    std::string propertyPath;
    core::TimelineTime time{0};
    double value{0.0};
    KeyframeInterpolation interpolation{KeyframeInterpolation::Linear};

    bool operator==(const GroupKeyframeRef& other) const noexcept {
        return propertyPath == other.propertyPath && time == other.time && value == other.value && interpolation == other.interpolation;
    }
};

struct ElementKeyframeInfo {
    std::string propertyPath;
    core::TimelineTime time{0};
    double value{0.0};
    KeyframeInterpolation interpolation{KeyframeInterpolation::Linear};

    bool operator==(const ElementKeyframeInfo& other) const noexcept {
        return propertyPath == other.propertyPath && time == other.time && value == other.value && interpolation == other.interpolation;
    }
};

class AnimationKeyframeQueryEngine {
public:
    static const std::vector<std::string>& getAllAnimationPropertyPaths();
    static bool isRecognizedPropertyPath(const std::string& propertyPath);

    static std::vector<std::string> getPropertyPathsForGroup(const std::string& group);

    static std::vector<GroupKeyframeRef> getGroupKeyframesAtTime(
        const std::unordered_map<std::string, AnimationChannel>& channels,
        const std::string& group,
        core::TimelineTime time,
        core::TimelineTime threshold = core::TimelineTime::fromTicks(100)
    );

    static bool hasGroupKeyframeAtTime(
        const std::unordered_map<std::string, AnimationChannel>& channels,
        const std::string& group,
        core::TimelineTime time,
        core::TimelineTime threshold = core::TimelineTime::fromTicks(100)
    );

    static std::vector<ElementKeyframeInfo> getElementKeyframes(
        const std::unordered_map<std::string, AnimationChannel>& channels
    );

    static bool hasKeyframesForPath(
        const std::unordered_map<std::string, AnimationChannel>& channels,
        const std::string& propertyPath
    );

    static std::optional<ElementKeyframeInfo> getKeyframeAtTime(
        const std::unordered_map<std::string, AnimationChannel>& channels,
        const std::string& propertyPath,
        core::TimelineTime time,
        core::TimelineTime threshold = core::TimelineTime::fromTicks(100)
    );
};

} // namespace catchim::editor
