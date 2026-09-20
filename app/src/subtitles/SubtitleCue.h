#pragma once

#include "core/time/TimelineTime.h"
#include <string>

namespace catchim::subtitles {

struct SubtitleCue {
    int32_t index = 0;
    core::TimelineTime startTime = core::TimelineTime(0);
    core::TimelineTime duration = core::TimelineTime(0);
    std::string text;

    [[nodiscard]] core::TimelineTime endTime() const noexcept {
        return startTime + duration;
    }
};

} // namespace catchim::subtitles
