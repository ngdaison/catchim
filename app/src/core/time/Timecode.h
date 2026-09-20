#pragma once

#include "TimelineTime.h"
#include <string>
#include <optional>

namespace catchim::core {

enum class TimecodeFormat {
    MM_SS,          // 05:30
    HH_MM_SS,       // 01:25:30
    HH_MM_SS_CS,    // 01:25:30:50 (Centiseconds)
    HH_MM_SS_FF     // 01:25:30:15 (Frames)
};

class Timecode {
public:
    static std::string format(
        TimelineTime time,
        TimecodeFormat format = TimecodeFormat::HH_MM_SS_FF,
        const FrameRate& rate = {30, 1}
    );

    static std::optional<TimelineTime> parse(
        std::string_view timecodeStr,
        TimecodeFormat format = TimecodeFormat::HH_MM_SS_FF,
        const FrameRate& rate = {30, 1}
    );

    static std::optional<TimecodeFormat> guessFormat(std::string_view timecodeStr);
};

} // namespace catchim::core
