#pragma once

#include "editor/timeline/Clip.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <string>

namespace catchim::editor {

class TimelineElementFactory {
public:
    static constexpr double kDefaultDurationSeconds = 5.0;

    static Clip buildGraphicElement(
        const core::ClipId& clipId,
        const std::string& graphicType = "rectangle",
        core::TimelineTime startTime = core::TimelineTime(0),
        core::TimelineTime duration = core::TimelineTime::fromSeconds(kDefaultDurationSeconds)
    );

    static Clip buildStickerElement(
        const core::ClipId& clipId,
        const std::string& stickerId,
        core::TimelineTime startTime = core::TimelineTime(0),
        core::TimelineTime duration = core::TimelineTime::fromSeconds(kDefaultDurationSeconds)
    );

    static Clip buildEffectElement(
        const core::ClipId& clipId,
        const std::string& effectType,
        core::TimelineTime startTime = core::TimelineTime(0),
        core::TimelineTime duration = core::TimelineTime::fromSeconds(kDefaultDurationSeconds)
    );

    // Type introspection
    static bool canElementHaveAudio(ClipType type) noexcept;
    static bool isVisualElement(ClipType type) noexcept;
    static bool isMaskableElement(ClipType type) noexcept;
    static bool isRetimableElement(ClipType type) noexcept;
    static bool canElementBeHidden(ClipType type) noexcept;
    static bool requiresMediaId(ClipType type) noexcept;
};

} // namespace catchim::editor
