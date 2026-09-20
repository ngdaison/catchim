#include "editor/animation/AnimationKeyframeQueryEngine.h"
#include <algorithm>

namespace catchim::editor {

const std::vector<std::string>& AnimationKeyframeQueryEngine::getAllAnimationPropertyPaths() {
    static const std::vector<std::string> paths = {
        "transform.positionX",
        "transform.positionY",
        "transform.scaleX",
        "transform.scaleY",
        "transform.rotate",
        "opacity",
        "volume",
        "color",
        "background.color",
        "background.paddingX",
        "background.paddingY",
        "background.offsetX",
        "background.offsetY",
        "background.cornerRadius"
    };
    return paths;
}

bool AnimationKeyframeQueryEngine::isRecognizedPropertyPath(const std::string& propertyPath) {
    const auto& allPaths = getAllAnimationPropertyPaths();
    return std::find(allPaths.begin(), allPaths.end(), propertyPath) != allPaths.end();
}

std::vector<std::string> AnimationKeyframeQueryEngine::getPropertyPathsForGroup(const std::string& group) {
    if (group == "transform.scale") {
        return {"transform.scaleX", "transform.scaleY"};
    }
    if (group == "transform.position") {
        return {"transform.positionX", "transform.positionY"};
    }
    if (group == "background.padding") {
        return {"background.paddingX", "background.paddingY"};
    }
    if (group == "background.offset") {
        return {"background.offsetX", "background.offsetY"};
    }
    return {};
}

std::vector<GroupKeyframeRef> AnimationKeyframeQueryEngine::getGroupKeyframesAtTime(
    const std::unordered_map<std::string, AnimationChannel>& channels,
    const std::string& group,
    core::TimelineTime time,
    core::TimelineTime threshold
) {
    const auto paths = getPropertyPathsForGroup(group);
    std::vector<GroupKeyframeRef> result;

    for (const auto& path : paths) {
        const auto it = channels.find(path);
        if (it != channels.end()) {
            const auto kfOpt = it->second.findKeyframeAt(time, threshold);
            if (kfOpt.has_value()) {
                result.push_back(GroupKeyframeRef{
                    .propertyPath = path,
                    .time = kfOpt->time,
                    .value = kfOpt->value,
                    .interpolation = kfOpt->interpolation
                });
            }
        }
    }

    return result;
}

bool AnimationKeyframeQueryEngine::hasGroupKeyframeAtTime(
    const std::unordered_map<std::string, AnimationChannel>& channels,
    const std::string& group,
    core::TimelineTime time,
    core::TimelineTime threshold
) {
    return !getGroupKeyframesAtTime(channels, group, time, threshold).empty();
}

std::vector<ElementKeyframeInfo> AnimationKeyframeQueryEngine::getElementKeyframes(
    const std::unordered_map<std::string, AnimationChannel>& channels
) {
    std::vector<ElementKeyframeInfo> result;

    for (const auto& [path, channel] : channels) {
        for (const auto& kf : channel.keyframes()) {
            result.push_back(ElementKeyframeInfo{
                .propertyPath = path,
                .time = kf.time,
                .value = kf.value,
                .interpolation = kf.interpolation
            });
        }
    }

    std::sort(result.begin(), result.end(), [](const ElementKeyframeInfo& a, const ElementKeyframeInfo& b) {
        if (a.time != b.time) {
            return a.time < b.time;
        }
        return a.propertyPath < b.propertyPath;
    });

    return result;
}

bool AnimationKeyframeQueryEngine::hasKeyframesForPath(
    const std::unordered_map<std::string, AnimationChannel>& channels,
    const std::string& propertyPath
) {
    const auto it = channels.find(propertyPath);
    return it != channels.end() && !it->second.empty();
}

std::optional<ElementKeyframeInfo> AnimationKeyframeQueryEngine::getKeyframeAtTime(
    const std::unordered_map<std::string, AnimationChannel>& channels,
    const std::string& propertyPath,
    core::TimelineTime time,
    core::TimelineTime threshold
) {
    const auto it = channels.find(propertyPath);
    if (it == channels.end()) {
        return std::nullopt;
    }

    const auto kfOpt = it->second.findKeyframeAt(time, threshold);
    if (!kfOpt.has_value()) {
        return std::nullopt;
    }

    return ElementKeyframeInfo{
        .propertyPath = propertyPath,
        .time = kfOpt->time,
        .value = kfOpt->value,
        .interpolation = kfOpt->interpolation
    };
}

} // namespace catchim::editor
