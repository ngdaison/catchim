#include "EffectKeyframeCommands.h"
#include <algorithm>

namespace catchim::editor {

// ============================================================================
// UpsertEffectParamKeyframeCommand
// ============================================================================

UpsertEffectParamKeyframeCommand::UpsertEffectParamKeyframeCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string effectId,
    std::string paramKey,
    core::TimelineTime time,
    double value,
    KeyframeInterpolation interpolation,
    KeyframeHandle leftHandle,
    KeyframeHandle rightHandle,
    double bezierX1,
    double bezierY1,
    double bezierX2,
    double bezierY2
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , effectId_(std::move(effectId))
    , paramKey_(std::move(paramKey))
    , propertyPath_(EffectParamAnimationEngine::buildEffectParamPath(effectId_, paramKey_))
    , time_(time)
    , value_(value)
    , interpolation_(interpolation)
    , leftHandle_(leftHandle)
    , rightHandle_(rightHandle)
    , bezierX1_(bezierX1)
    , bezierY1_(bezierY1)
    , bezierX2_(bezierX2)
    , bezierY2_(bezierY2)
{
}

bool UpsertEffectParamKeyframeCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }

    // Check if effect exists on clip
    const auto& effs = clip->effects();
    auto it = std::find_if(effs.begin(), effs.end(), [&](const EffectInstance& e) {
        return e.id == effectId_;
    });
    if (it == effs.end()) {
        return false;
    }

    // Save channel state before mutating
    hadChannel_ = clip->hasAnimationChannel(propertyPath_);
    if (hadChannel_) {
        savedChannel_ = *clip->findAnimationChannel(propertyPath_);
    } else {
        savedChannel_ = std::nullopt;
    }

    // Clamp time within [0, duration]
    core::TimelineTime boundedTime = std::clamp(time_, core::TimelineTime(0), clip->duration());

    double defaultVal = it->params.value(paramKey_, 0.0);
    auto& channel = clip->getOrCreateAnimationChannel(propertyPath_, defaultVal);

    Keyframe kf{boundedTime, value_, interpolation_};
    kf.leftHandle = leftHandle_;
    kf.rightHandle = rightHandle_;
    kf.bezierX1 = bezierX1_;
    kf.bezierY1 = bezierY1_;
    kf.bezierX2 = bezierX2_;
    kf.bezierY2 = bezierY2_;

    channel.addOrUpdateKeyframe(std::move(kf));
    executed_ = true;
    return true;
}

bool UpsertEffectParamKeyframeCommand::undo() {
    if (!executed_) {
        return false;
    }

    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }

    if (hadChannel_ && savedChannel_.has_value()) {
        clip->animationChannels()[propertyPath_] = *savedChannel_;
    } else {
        clip->removeAnimationChannel(propertyPath_);
    }

    executed_ = false;
    return true;
}

// ============================================================================
// RemoveEffectParamKeyframeCommand
// ============================================================================

RemoveEffectParamKeyframeCommand::RemoveEffectParamKeyframeCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string effectId,
    std::string paramKey,
    core::TimelineTime time,
    core::TimelineTime threshold
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , effectId_(std::move(effectId))
    , paramKey_(std::move(paramKey))
    , propertyPath_(EffectParamAnimationEngine::buildEffectParamPath(effectId_, paramKey_))
    , time_(time)
    , threshold_(threshold)
{
}

bool RemoveEffectParamKeyframeCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }

    auto* channel = clip->findAnimationChannel(propertyPath_);
    if (!channel) {
        return false;
    }

    savedChannel_ = *channel;
    bool removed = channel->removeKeyframeAt(time_, threshold_);
    if (!removed) {
        return false;
    }

    executed_ = true;
    return true;
}

bool RemoveEffectParamKeyframeCommand::undo() {
    if (!executed_ || !savedChannel_.has_value()) {
        return false;
    }

    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }

    clip->animationChannels()[propertyPath_] = *savedChannel_;
    executed_ = false;
    return true;
}

} // namespace catchim::editor
