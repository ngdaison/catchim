#include "KeyframeCommands.h"
#include <algorithm>

namespace catchim::editor {

// ==========================================
// UpsertKeyframeCommand
// ==========================================

UpsertKeyframeCommand::UpsertKeyframeCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string propertyPath,
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
    , propertyPath_(std::move(propertyPath))
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

bool UpsertKeyframeCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    if (!executed_) {
        hadChannel_ = clip->hasAnimationChannel(propertyPath_);
        if (hadChannel_) {
            savedChannel_ = *clip->findAnimationChannel(propertyPath_);
        }
    }

    core::TimelineTime boundedTime = time_;
    if (boundedTime < core::TimelineTime(0)) {
        boundedTime = core::TimelineTime(0);
    } else if (boundedTime > clip->duration()) {
        boundedTime = clip->duration();
    }

    auto& channel = clip->getOrCreateAnimationChannel(propertyPath_, value_);

    Keyframe kf;
    kf.time = boundedTime;
    kf.value = value_;
    kf.interpolation = interpolation_;
    kf.leftHandle = leftHandle_;
    kf.rightHandle = rightHandle_;
    kf.bezierX1 = bezierX1_;
    kf.bezierY1 = bezierY1_;
    kf.bezierX2 = bezierX2_;
    kf.bezierY2 = bezierY2_;

    channel.addOrUpdateKeyframe(kf);
    executed_ = true;
    return true;
}

bool UpsertKeyframeCommand::undo() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    if (hadChannel_ && savedChannel_.has_value()) {
        clip->animationChannels()[propertyPath_] = *savedChannel_;
    } else {
        clip->removeAnimationChannel(propertyPath_);
    }
    return true;
}

// ==========================================
// RemoveKeyframeCommand
// ==========================================

RemoveKeyframeCommand::RemoveKeyframeCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string propertyPath,
    core::TimelineTime time
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , propertyPath_(std::move(propertyPath))
    , time_(time)
{
}

bool RemoveKeyframeCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    hadChannel_ = clip->hasAnimationChannel(propertyPath_);
    if (!hadChannel_) return false;

    if (!executed_) {
        savedChannel_ = *clip->findAnimationChannel(propertyPath_);
    }

    auto* chan = clip->findAnimationChannel(propertyPath_);
    if (!chan) return false;

    bool removed = chan->removeKeyframeAt(time_);
    if (removed) {
        executed_ = true;
    }
    return removed;
}

bool RemoveKeyframeCommand::undo() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    if (savedChannel_.has_value()) {
        clip->animationChannels()[propertyPath_] = *savedChannel_;
    }
    return true;
}

// ==========================================
// RetimeKeyframeCommand
// ==========================================

RetimeKeyframeCommand::RetimeKeyframeCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string propertyPath,
    core::TimelineTime oldTime,
    core::TimelineTime newTime
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , propertyPath_(std::move(propertyPath))
    , oldTime_(oldTime)
    , newTime_(newTime)
{
}

bool RetimeKeyframeCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    hadChannel_ = clip->hasAnimationChannel(propertyPath_);
    if (!hadChannel_) return false;

    if (!executed_) {
        savedChannel_ = *clip->findAnimationChannel(propertyPath_);
    }

    auto* chan = clip->findAnimationChannel(propertyPath_);
    if (!chan) return false;

    auto kfOpt = chan->findKeyframeAt(oldTime_);
    if (!kfOpt.has_value()) return false;

    Keyframe kf = *kfOpt;
    chan->removeKeyframeAt(oldTime_);

    core::TimelineTime boundedTime = newTime_;
    if (boundedTime < core::TimelineTime(0)) {
        boundedTime = core::TimelineTime(0);
    } else if (boundedTime > clip->duration()) {
        boundedTime = clip->duration();
    }
    kf.time = boundedTime;

    chan->addOrUpdateKeyframe(kf);
    executed_ = true;
    return true;
}

bool RetimeKeyframeCommand::undo() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    if (savedChannel_.has_value()) {
        clip->animationChannels()[propertyPath_] = *savedChannel_;
    }
    return true;
}

// ==========================================
// UpdateKeyframeCurveCommand
// ==========================================

UpdateKeyframeCurveCommand::UpdateKeyframeCurveCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string propertyPath,
    core::TimelineTime time,
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
    , propertyPath_(std::move(propertyPath))
    , time_(time)
    , interpolation_(interpolation)
    , leftHandle_(leftHandle)
    , rightHandle_(rightHandle)
    , bezierX1_(bezierX1)
    , bezierY1_(bezierY1)
    , bezierX2_(bezierX2)
    , bezierY2_(bezierY2)
{
}

bool UpdateKeyframeCurveCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    hadChannel_ = clip->hasAnimationChannel(propertyPath_);
    if (!hadChannel_) return false;

    if (!executed_) {
        savedChannel_ = *clip->findAnimationChannel(propertyPath_);
    }

    auto* chan = clip->findAnimationChannel(propertyPath_);
    if (!chan) return false;

    auto kfOpt = chan->findKeyframeAt(time_);
    if (!kfOpt.has_value()) return false;

    Keyframe kf = *kfOpt;
    kf.interpolation = interpolation_;
    kf.leftHandle = leftHandle_;
    kf.rightHandle = rightHandle_;
    kf.bezierX1 = bezierX1_;
    kf.bezierY1 = bezierY1_;
    kf.bezierX2 = bezierX2_;
    kf.bezierY2 = bezierY2_;

    chan->addOrUpdateKeyframe(kf);
    executed_ = true;
    return true;
}

bool UpdateKeyframeCurveCommand::undo() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) return false;

    if (savedChannel_.has_value()) {
        clip->animationChannels()[propertyPath_] = *savedChannel_;
    }
    return true;
}

} // namespace catchim::editor
