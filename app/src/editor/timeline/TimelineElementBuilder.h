#pragma once

#include "editor/timeline/Clip.h"
#include "media/MediaAsset.h"
#include "editor/params/ElementParamRegistry.h"
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace catchim::editor {

class TimelineElementBuilder {
public:
    static constexpr double kDefaultNewElementDurationSeconds = 5.0;

    // Constructs a new Clip instance representing media (video, audio, image) with registered defaults
    static Clip buildElementFromMedia(
        const core::ClipId& clipId,
        const core::MediaId& mediaId,
        media::MediaType mediaType,
        std::string name,
        core::TimelineTime duration,
        core::TimelineTime startTime = core::TimelineTime(0)
    );

    // Constructs a new Text Clip populated with typography and visual defaults
    static Clip buildTextElement(
        const core::ClipId& clipId,
        std::string name = "Text",
        std::string content = "Title",
        core::TimelineTime startTime = core::TimelineTime(0),
        core::TimelineTime duration = core::TimelineTime::fromSeconds(kDefaultNewElementDurationSeconds),
        const nlohmann::json& overrides = nlohmann::json::object()
    );

    // Merges override parameters on top of a base parameter JSON object
    static nlohmann::json mergeParamValues(
        const nlohmann::json& base,
        const nlohmann::json& overrides
    );
};

} // namespace catchim::editor
