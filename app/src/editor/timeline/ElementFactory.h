#pragma once

#include "Clip.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace catchim::editor {

/**
 * @brief Factory for standard timeline elements/clips.
 * Corresponds to web/src/timeline/creation.ts and element creation in web/src/timeline/element-utils.ts.
 */
class ElementFactory {
public:
    // Default duration for new non-media elements (5 seconds = 600,000 ticks)
    static inline const core::TimelineTime DEFAULT_NEW_ELEMENT_DURATION = core::TimelineTime::fromSeconds(5.0);

    static Clip buildTextElement(
        const std::string& name,
        const std::string& content,
        core::TimelineTime startTime,
        core::TimelineTime duration = DEFAULT_NEW_ELEMENT_DURATION,
        double fontSize = 48.0,
        const std::string& fontFamily = "Inter",
        const std::string& color = "#FFFFFF"
    );

    static Clip buildVideoElement(
        const core::MediaId& mediaId,
        const std::string& name,
        core::TimelineTime duration,
        core::TimelineTime startTime,
        std::optional<core::TimelineTime> sourceDuration = std::nullopt
    );

    static Clip buildAudioElement(
        const core::MediaId& mediaId,
        const std::string& name,
        core::TimelineTime duration,
        core::TimelineTime startTime,
        std::optional<core::TimelineTime> sourceDuration = std::nullopt
    );

    static Clip buildImageElement(
        const core::MediaId& mediaId,
        const std::string& name,
        core::TimelineTime duration,
        core::TimelineTime startTime
    );

    static Clip buildStickerElement(
        const std::string& stickerId,
        const std::string& name,
        core::TimelineTime startTime,
        core::TimelineTime duration = DEFAULT_NEW_ELEMENT_DURATION
    );

    static Clip buildGraphicElement(
        const std::string& definitionId,
        const std::string& name,
        core::TimelineTime startTime,
        core::TimelineTime duration = DEFAULT_NEW_ELEMENT_DURATION,
        const nlohmann::json& customParams = nlohmann::json::object()
    );

    static Clip buildEffectElement(
        const std::string& effectType,
        const std::string& name,
        core::TimelineTime startTime,
        core::TimelineTime duration = DEFAULT_NEW_ELEMENT_DURATION,
        const nlohmann::json& customParams = nlohmann::json::object()
    );

    static Clip buildElementFromMedia(
        const core::MediaId& mediaId,
        const std::string& mediaType, // "video", "audio", "image"
        const std::string& name,
        core::TimelineTime duration,
        core::TimelineTime startTime
    );
};

} // namespace catchim::editor
