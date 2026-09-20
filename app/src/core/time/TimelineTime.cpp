#include "TimelineTime.h"

namespace catchim::core {

namespace {
inline int64_t getTicksPerFrame(const FrameRate& rate) noexcept {
    if (rate.numerator <= 0 || rate.denominator <= 0) return 4000; // default 30fps
    return (TICKS_PER_SECOND * static_cast<int64_t>(rate.denominator)) / rate.numerator;
}

inline int64_t divEuclid(int64_t a, int64_t b) noexcept {
    int64_t q = a / b;
    int64_t r = a % b;
    return (r < 0) ? q + (b > 0 ? -1 : 1) : q;
}

inline int64_t remEuclid(int64_t a, int64_t b) noexcept {
    int64_t r = a % b;
    return (r < 0) ? r + (b > 0 ? b : -b) : r;
}
} // namespace

TimelineTime TimelineTime::roundToFrame(const FrameRate& rate) const noexcept {
    int64_t tpf = getTicksPerFrame(rate);
    if (tpf <= 0) return *this;

    int64_t remainder = remEuclid(ticks_, tpf);
    int64_t floorVal = divEuclid(ticks_, tpf);
    int64_t frame = (remainder * 2 >= tpf) ? floorVal + 1 : floorVal;
    return TimelineTime(frame * tpf);
}

TimelineTime TimelineTime::floorToFrame(const FrameRate& rate) const noexcept {
    int64_t tpf = getTicksPerFrame(rate);
    if (tpf <= 0) return *this;

    int64_t floorVal = divEuclid(ticks_, tpf);
    return TimelineTime(floorVal * tpf);
}

TimelineTime TimelineTime::lastFrameTime(TimelineTime duration, const FrameRate& rate) noexcept {
    if (duration.ticks() <= 0) return TimelineTime(0);
    int64_t lastTick = duration.ticks() - 1;
    return TimelineTime(lastTick).floorToFrame(rate);
}

TimelineTime TimelineTime::snappedSeek(TimelineTime duration, const FrameRate& rate) const noexcept {
    TimelineTime snapped = roundToFrame(rate);
    return snapped.clamp(TimelineTime(0), duration);
}

} // namespace catchim::core
