// Feature 224 -- mirrors web/src/wasm/media-time.ts
#include "core/time/MediaTimeUtils.h"
#include <algorithm>
#include <cmath>

namespace catchim::core {

int64_t MediaTimeUtils::roundMediaTime(double time) {
    double roundedMagnitude = std::round(std::abs(time));
    if (roundedMagnitude == 0.0) {
        return 0LL;
    }
    return static_cast<int64_t>(time < 0.0 ? -roundedMagnitude : roundedMagnitude);
}

int64_t MediaTimeUtils::addMediaTime(int64_t a, int64_t b) noexcept {
    return a + b;
}

int64_t MediaTimeUtils::subMediaTime(int64_t a, int64_t b) noexcept {
    return a - b;
}

int64_t MediaTimeUtils::maxMediaTime(int64_t a, int64_t b) noexcept {
    return (a > b) ? a : b;
}

int64_t MediaTimeUtils::minMediaTime(int64_t a, int64_t b) noexcept {
    return (a < b) ? a : b;
}

int64_t MediaTimeUtils::clampMediaTime(int64_t time, int64_t minVal, int64_t maxVal) noexcept {
    if (time < minVal) return minVal;
    if (time > maxVal) return maxVal;
    return time;
}

TimelineTime MediaTimeUtils::roundFrameTime(TimelineTime time, FrameRate fps) {
    return time.roundToFrame(fps);
}

int64_t MediaTimeUtils::roundFrameTicks(int64_t ticks, FrameRate fps) {
    return TimelineTime(ticks).roundToFrame(fps).ticks();
}

TimelineTime MediaTimeUtils::snapSeekMediaTime(TimelineTime time, TimelineTime duration, FrameRate fps) {
    TimelineTime rounded = time.roundToFrame(fps);
    TimelineTime maxSeek = lastFrameMediaTime(duration, fps);
    if (rounded.ticks() < 0) return TimelineTime(0);
    if (rounded.ticks() > maxSeek.ticks()) return maxSeek;
    return rounded;
}

TimelineTime MediaTimeUtils::lastFrameMediaTime(TimelineTime duration, FrameRate fps) {
    if (duration.ticks() <= 0) return TimelineTime(0);
    TimelineTime frameDur = fps.frameDuration();
    TimelineTime last = duration - frameDur;
    if (last.ticks() < 0) return TimelineTime(0);
    return last.floorToFrame(fps);
}

} // namespace catchim::core
