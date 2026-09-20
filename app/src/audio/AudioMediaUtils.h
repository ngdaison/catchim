#pragma once

#include "editor/timeline/Timeline.h"
#include "core/ids/Ids.h"
#include <vector>

namespace catchim::audio {

class AudioMediaUtils {
public:
    static void downmixStereo(
        const float* left,
        const float* right,
        float* out,
        size_t length) noexcept;

    static bool timelineHasAudio(const editor::Timeline& timeline) noexcept;

    static std::vector<core::ClipId> collectAudibleClips(const editor::Timeline& timeline);

    static double dBToLinear(double db) noexcept;
    static double linearToDb(double linear) noexcept;
};

} // namespace catchim::audio
