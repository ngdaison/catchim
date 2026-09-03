#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace opencut::subtitles {

struct SubtitleCue {
    int32_t index = 0;
    int64_t start_time = 0;
    int64_t duration = 0;
    std::string text;

    [[nodiscard]] int64_t end_time() const noexcept { return start_time + duration; }
};

class SrtParser {
public:
    static std::vector<SubtitleCue> parse(std::string_view srt_content);
    static std::string format(const std::vector<SubtitleCue>& cues);
    static int64_t parse_timestamp(std::string_view ts);
    static std::string format_timestamp(int64_t ticks);
};

} // namespace opencut::subtitles
