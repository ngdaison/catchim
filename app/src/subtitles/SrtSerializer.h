#pragma once

#include "subtitles/SubtitleCue.h"
#include <vector>
#include <string>

namespace catchim::subtitles {

class SrtSerializer {
public:
    static std::string serialize(const std::vector<SubtitleCue>& cues);
    static std::string formatTimestamp(core::TimelineTime time);
};

} // namespace catchim::subtitles
