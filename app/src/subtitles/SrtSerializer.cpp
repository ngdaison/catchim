#include "subtitles/SrtSerializer.h"
#include <sstream>
#include <iomanip>

namespace catchim::subtitles {

std::string SrtSerializer::formatTimestamp(core::TimelineTime time) {
    int64_t ticks = std::max<int64_t>(0, time.ticks());

    constexpr int64_t TICKS_PER_HOUR = 3600LL * 120000LL;
    constexpr int64_t TICKS_PER_MINUTE = 60LL * 120000LL;
    constexpr int64_t TICKS_PER_SECOND = 120000LL;
    constexpr int64_t TICKS_PER_MILLI = 120LL;

    int64_t hours = ticks / TICKS_PER_HOUR;
    int64_t rem = ticks % TICKS_PER_HOUR;

    int64_t minutes = rem / TICKS_PER_MINUTE;
    rem = rem % TICKS_PER_MINUTE;

    int64_t seconds = rem / TICKS_PER_SECOND;
    rem = rem % TICKS_PER_SECOND;

    int64_t millis = rem / TICKS_PER_MILLI;

    std::ostringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << hours << ":"
       << std::setw(2) << minutes << ":"
       << std::setw(2) << seconds << ","
       << std::setw(3) << millis;

    return ss.str();
}

std::string SrtSerializer::serialize(const std::vector<SubtitleCue>& cues) {
    std::ostringstream ss;
    int32_t idx = 1;

    for (const auto& cue : cues) {
        ss << idx << "\n";
        ss << formatTimestamp(cue.startTime) << " --> " << formatTimestamp(cue.endTime()) << "\n";
        ss << cue.text << "\n\n";
        idx++;
    }

    return ss.str();
}

} // namespace catchim::subtitles
