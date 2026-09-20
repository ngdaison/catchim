#include "ElementFactory.h"

namespace catchim::editor {

Clip ElementFactory::buildTextElement(
    const std::string& name,
    const std::string& content,
    core::TimelineTime startTime,
    core::TimelineTime duration,
    double fontSize,
    const std::string& fontFamily,
    const std::string& color
) {
    Clip clip(
        core::ClipId::generate(),
        ClipType::Text,
        name.empty() ? "Text" : name,
        startTime,
        duration
    );
    clip.setParam("content", content);
    clip.setParam("fontSize", fontSize);
    clip.setParam("fontFamily", fontFamily);
    clip.setParam("color", color);
    clip.setParam("textAlign", "center");
    return clip;
}

Clip ElementFactory::buildVideoElement(
    const core::MediaId& mediaId,
    const std::string& name,
    core::TimelineTime duration,
    core::TimelineTime startTime,
    std::optional<core::TimelineTime> sourceDuration
) {
    Clip clip(
        core::ClipId::generate(),
        ClipType::Video,
        name,
        startTime,
        duration
    );
    clip.setMediaId(mediaId);
    clip.setSourceDuration(sourceDuration.has_value() ? sourceDuration : std::optional<core::TimelineTime>(duration));
    clip.setParam("volume", 1.0);
    clip.setParam("muted", false);
    clip.setParam("isSourceAudioEnabled", true);
    return clip;
}

Clip ElementFactory::buildAudioElement(
    const core::MediaId& mediaId,
    const std::string& name,
    core::TimelineTime duration,
    core::TimelineTime startTime,
    std::optional<core::TimelineTime> sourceDuration
) {
    Clip clip(
        core::ClipId::generate(),
        ClipType::Audio,
        name,
        startTime,
        duration
    );
    clip.setMediaId(mediaId);
    clip.setSourceDuration(sourceDuration.has_value() ? sourceDuration : std::optional<core::TimelineTime>(duration));
    clip.setParam("volume", 1.0);
    clip.setParam("muted", false);
    clip.setParam("sourceType", "upload");
    return clip;
}

Clip ElementFactory::buildImageElement(
    const core::MediaId& mediaId,
    const std::string& name,
    core::TimelineTime duration,
    core::TimelineTime startTime
) {
    Clip clip(
        core::ClipId::generate(),
        ClipType::Image,
        name,
        startTime,
        duration
    );
    clip.setMediaId(mediaId);
    clip.setSourceDuration(duration);
    return clip;
}

Clip ElementFactory::buildStickerElement(
    const std::string& stickerId,
    const std::string& name,
    core::TimelineTime startTime,
    core::TimelineTime duration
) {
    std::string displayName = name;
    if (displayName.empty()) {
        auto colonPos = stickerId.find_last_of(':');
        if (colonPos != std::string::npos && colonPos + 1 < stickerId.size()) {
            displayName = stickerId.substr(colonPos + 1);
        } else {
            displayName = stickerId;
        }
    }

    Clip clip(
        core::ClipId::generate(),
        ClipType::Sticker,
        displayName,
        startTime,
        duration
    );
    clip.setParam("stickerId", stickerId);
    return clip;
}

Clip ElementFactory::buildGraphicElement(
    const std::string& definitionId,
    const std::string& name,
    core::TimelineTime startTime,
    core::TimelineTime duration,
    const nlohmann::json& customParams
) {
    Clip clip(
        core::ClipId::generate(),
        ClipType::Graphic,
        name.empty() ? definitionId : name,
        startTime,
        duration
    );
    clip.setParam("definitionId", definitionId);
    if (customParams.is_object()) {
        for (auto it = customParams.begin(); it != customParams.end(); ++it) {
            clip.params()[it.key()] = it.value();
        }
    }
    return clip;
}

Clip ElementFactory::buildEffectElement(
    const std::string& effectType,
    const std::string& name,
    core::TimelineTime startTime,
    core::TimelineTime duration,
    const nlohmann::json& customParams
) {
    Clip clip(
        core::ClipId::generate(),
        ClipType::Effect,
        name.empty() ? effectType : name,
        startTime,
        duration
    );
    clip.setParam("effectType", effectType);
    if (customParams.is_object()) {
        for (auto it = customParams.begin(); it != customParams.end(); ++it) {
            clip.params()[it.key()] = it.value();
        }
    }
    return clip;
}

Clip ElementFactory::buildElementFromMedia(
    const core::MediaId& mediaId,
    const std::string& mediaType,
    const std::string& name,
    core::TimelineTime duration,
    core::TimelineTime startTime
) {
    if (mediaType == "audio") {
        return buildAudioElement(mediaId, name, duration, startTime);
    } else if (mediaType == "image") {
        return buildImageElement(mediaId, name, duration, startTime);
    }
    // Default to video
    return buildVideoElement(mediaId, name, duration, startTime);
}

} // namespace catchim::editor
