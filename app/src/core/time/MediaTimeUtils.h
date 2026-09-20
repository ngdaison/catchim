#pragma once
// Feature 224 -- mirrors web/src/wasm/media-time.ts
#include "core/time/TimelineTime.h"
#include <cstdint>

namespace catchim::core {

class MediaTimeUtils {
public:
    static int64_t roundMediaTime(double time);
    static int64_t addMediaTime(int64_t a, int64_t b) noexcept;
    static int64_t subMediaTime(int64_t a, int64_t b) noexcept;
    static int64_t maxMediaTime(int64_t a, int64_t b) noexcept;
    static int64_t minMediaTime(int64_t a, int64_t b) noexcept;
    static int64_t clampMediaTime(int64_t time, int64_t minVal, int64_t maxVal) noexcept;

    static TimelineTime roundFrameTime(TimelineTime time, FrameRate fps);
    static int64_t roundFrameTicks(int64_t ticks, FrameRate fps);

    static TimelineTime snapSeekMediaTime(TimelineTime time, TimelineTime duration, FrameRate fps);
    static TimelineTime lastFrameMediaTime(TimelineTime duration, FrameRate fps);
};

} // namespace catchim::core
