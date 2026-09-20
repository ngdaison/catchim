#pragma once

#include "subtitles/SubtitleCue.h"
#include <vector>
#include <string>
#include <string_view>
#include <optional>

namespace catchim::subtitles {

struct ParseResult {
    std::vector<SubtitleCue> cues;
    size_t skippedCount = 0;
    std::vector<std::string> warnings;
};

class SrtParser {
public:
    static ParseResult parse(std::string_view input);
    static std::optional<core::TimelineTime> parseTimestamp(std::string_view ts);
};

} // namespace catchim::subtitles
