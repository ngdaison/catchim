#include "editor/timeline/TimelineElementBuilder.h"

namespace catchim::editor {

nlohmann::json TimelineElementBuilder::mergeParamValues(
    const nlohmann::json& base,
    const nlohmann::json& overrides
) {
    nlohmann::json result = base.is_object() ? base : nlohmann::json::object();
    if (overrides.is_object()) {
        for (auto it = overrides.begin(); it != overrides.end(); ++it) {
            result[it.key()] = it.value();
        }
    }
    return result;
}

Clip TimelineElementBuilder::buildElementFromMedia(
    const core::ClipId& clipId,
    const core::MediaId& mediaId,
    media::MediaType mediaType,
    std::string name,
    core::TimelineTime duration,
    core::TimelineTime startTime
) {
    ClipType clipType = ClipType::Video;
    if (mediaType == media::MediaType::Audio) {
        clipType = ClipType::Audio;
    } else if (mediaType == media::MediaType::Image) {
        clipType = ClipType::Image;
    }

    Clip clip(
        clipId,
        clipType,
        std::move(name),
        startTime,
        duration,
        core::TimelineTime(0),
        core::TimelineTime(0)
    );

    clip.setSourceDuration(duration);
    clip.setMediaId(mediaId);
    clip.setParams(ElementParamRegistry::buildDefaultParamValues(clipType));

    return clip;
}

Clip TimelineElementBuilder::buildTextElement(
    const core::ClipId& clipId,
    std::string name,
    std::string content,
    core::TimelineTime startTime,
    core::TimelineTime duration,
    const nlohmann::json& overrides
) {
    Clip clip(
        clipId,
        ClipType::Text,
        std::move(name),
        startTime,
        duration,
        core::TimelineTime(0),
        core::TimelineTime(0)
    );

    nlohmann::json params = ElementParamRegistry::buildDefaultParamValues(ClipType::Text);
    params["content"] = std::move(content);

    if (!overrides.empty()) {
        params = mergeParamValues(params, overrides);
    }

    clip.setParams(std::move(params));
    return clip;
}

} // namespace catchim::editor
