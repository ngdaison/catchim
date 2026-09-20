#include "editor/animation/AnimationTransformResolver.h"
#include <algorithm>

namespace catchim::editor {

double AnimationTransformResolver::resolveChannelValue(
    const Clip& clip,
    const std::string& primaryKey,
    const std::string& aliasKey,
    core::TimelineTime localTime,
    double fallbackValue
) {
    // 1. Check primary channel (e.g. "position_x")
    const auto* channel = clip.findAnimationChannel(primaryKey);
    if (channel != nullptr && !channel->empty()) {
        return channel->getValueAt(localTime);
    }

    // 2. Check alias channel (e.g. "transform.positionX")
    if (!aliasKey.empty()) {
        const auto* aliasChan = clip.findAnimationChannel(aliasKey);
        if (aliasChan != nullptr && !aliasChan->empty()) {
            return aliasChan->getValueAt(localTime);
        }
    }

    // 3. Fallback to base parameter
    return clip.getParam<double>(primaryKey, fallbackValue);
}

AnimatedTransform AnimationTransformResolver::resolveTransformAtTime(
    const Clip& clip,
    core::TimelineTime localTime
) {
    core::TimelineTime safeTime(std::max<int64_t>(0, localTime.ticks()));

    double baseX = clip.getParam<double>("position_x", 0.0);
    double baseY = clip.getParam<double>("position_y", 0.0);
    double baseScaleX = clip.getParam<double>("scale_x", 1.0);
    double baseScaleY = clip.getParam<double>("scale_y", 1.0);
    double baseRotate = clip.getParam<double>("rotate", 0.0);

    return AnimatedTransform{
        .positionX = resolveChannelValue(clip, "position_x", "transform.positionX", safeTime, baseX),
        .positionY = resolveChannelValue(clip, "position_y", "transform.positionY", safeTime, baseY),
        .scaleX = resolveChannelValue(clip, "scale_x", "transform.scaleX", safeTime, baseScaleX),
        .scaleY = resolveChannelValue(clip, "scale_y", "transform.scaleY", safeTime, baseScaleY),
        .rotate = resolveChannelValue(clip, "rotate", "transform.rotate", safeTime, baseRotate)
    };
}

double AnimationTransformResolver::resolveOpacityAtTime(
    const Clip& clip,
    core::TimelineTime localTime
) {
    core::TimelineTime safeTime(std::max<int64_t>(0, localTime.ticks()));
    double baseOpacity = clip.getParam<double>("opacity", 1.0);
    double val = resolveChannelValue(clip, "opacity", "", safeTime, baseOpacity);
    return std::clamp(val, 0.0, 1.0);
}

double AnimationTransformResolver::resolveVolumeAtTime(
    const Clip& clip,
    core::TimelineTime localTime
) {
    core::TimelineTime safeTime(std::max<int64_t>(0, localTime.ticks()));
    double baseVolume = clip.getParam<double>("volume", 1.0);
    double val = resolveChannelValue(clip, "volume", "", safeTime, baseVolume);
    return std::clamp(val, 0.0, 2.0);
}

} // namespace catchim::editor
