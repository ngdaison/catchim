#include "Clip.h"

namespace catchim::editor {

Clip::Clip(
    core::ClipId id,
    ClipType type,
    std::string name,
    core::TimelineTime startTime,
    core::TimelineTime duration,
    core::TimelineTime trimStart,
    core::TimelineTime trimEnd
)
    : id_(std::move(id))
    , type_(type)
    , name_(std::move(name))
    , startTime_(startTime)
    , duration_(duration)
    , trimStart_(trimStart)
    , trimEnd_(trimEnd)
{
    // Default transform parameters
    params_["transform.positionX"] = 0.0;
    params_["transform.positionY"] = 0.0;
    params_["transform.scaleX"] = 1.0;
    params_["transform.scaleY"] = 1.0;
    params_["transform.rotate"] = 0.0;
    params_["opacity"] = 1.0;
    params_["blendMode"] = "normal";

    if (type == ClipType::Audio || type == ClipType::Video) {
        params_["volume"] = 1.0;
        params_["muted"] = false;
    }

    if (type == ClipType::Text) {
        params_["content"] = name_;
        params_["fontSize"] = 48.0;
        params_["fontFamily"] = "Inter";
        params_["color"] = "#FFFFFF";
        params_["textAlign"] = "center";
    }
}

Clip Clip::clone(core::ClipId newId) const {
    Clip c(
        std::move(newId),
        type_,
        name_,
        startTime_,
        duration_,
        trimStart_,
        trimEnd_
    );
    c.sourceDuration_ = sourceDuration_;
    c.mediaId_ = mediaId_;
    c.hidden_ = hidden_;
    c.muted_ = muted_;
    c.params_ = params_;
    c.animationChannels_ = animationChannels_;
    c.masks_ = masks_;
    c.effects_ = effects_;
    return c;
}

const AnimationChannel* Clip::findAnimationChannel(const std::string& property) const {
    auto it = animationChannels_.find(property);
    return it != animationChannels_.end() ? &it->second : nullptr;
}

AnimationChannel* Clip::findAnimationChannel(const std::string& property) {
    auto it = animationChannels_.find(property);
    return it != animationChannels_.end() ? &it->second : nullptr;
}

AnimationChannel& Clip::getOrCreateAnimationChannel(const std::string& property, double defaultValue) {
    auto it = animationChannels_.find(property);
    if (it != animationChannels_.end()) {
        return it->second;
    }
    auto [inserted, _] = animationChannels_.emplace(property, AnimationChannel(property, defaultValue));
    return inserted->second;
}

bool Clip::hasAnimationChannel(const std::string& property) const {
    return animationChannels_.find(property) != animationChannels_.end();
}

bool Clip::removeAnimationChannel(const std::string& property) {
    return animationChannels_.erase(property) > 0;
}

} // namespace catchim::editor
