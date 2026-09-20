#include "core/time/EuclideanFrameSnapper.h"
#include <algorithm>

namespace catchim::core {

int64_t EuclideanFrameSnapper::divEuclid(int64_t a, int64_t b) noexcept {
    if (b == 0) return 0;
    int64_t q = a / b;
    int64_t r = a % b;
    return (r < 0) ? q + (b > 0 ? -1 : 1) : q;
}

int64_t EuclideanFrameSnapper::remEuclid(int64_t a, int64_t b) noexcept {
    if (b == 0) return 0;
    int64_t r = a % b;
    return (r < 0) ? r + (b > 0 ? b : -b) : r;
}

std::optional<int64_t> EuclideanFrameSnapper::getTicksPerFrame(const FrameRate& rate) noexcept {
    if (rate.numerator <= 0 || rate.denominator <= 0) {
        return std::nullopt;
    }

    int64_t tickNum = TICKS_PER_SECOND * static_cast<int64_t>(rate.denominator);
    int64_t tickDen = static_cast<int64_t>(rate.numerator);

    if (tickNum % tickDen != 0) {
        // Not an exact integer divisor of ticks, return approximate integer
        return tickNum / tickDen;
    }

    return tickNum / tickDen;
}

std::optional<TimelineTime> EuclideanFrameSnapper::roundToFrame(
    TimelineTime time,
    const FrameRate& rate
) noexcept {
    auto tpfOpt = getTicksPerFrame(rate);
    if (!tpfOpt.has_value() || *tpfOpt <= 0) {
        return std::nullopt;
    }

    int64_t tpf = *tpfOpt;
    int64_t rem = remEuclid(time.ticks(), tpf);
    int64_t fl = divEuclid(time.ticks(), tpf);
    int64_t frame = (rem * 2 >= tpf) ? fl + 1 : fl;

    return TimelineTime(frame * tpf);
}

std::optional<TimelineTime> EuclideanFrameSnapper::floorToFrame(
    TimelineTime time,
    const FrameRate& rate
) noexcept {
    auto tpfOpt = getTicksPerFrame(rate);
    if (!tpfOpt.has_value() || *tpfOpt <= 0) {
        return std::nullopt;
    }

    int64_t tpf = *tpfOpt;
    int64_t fl = divEuclid(time.ticks(), tpf);

    return TimelineTime(fl * tpf);
}

TimelineTime EuclideanFrameSnapper::lastFrameTime(
    TimelineTime duration,
    const FrameRate& rate
) noexcept {
    if (duration.ticks() <= 0) {
        return TimelineTime(0);
    }

    int64_t lastTick = duration.ticks() - 1;
    auto floored = floorToFrame(TimelineTime(lastTick), rate);
    return floored.value_or(TimelineTime(0));
}

TimelineTime EuclideanFrameSnapper::snappedSeekTime(
    TimelineTime time,
    TimelineTime duration,
    const FrameRate& rate
) noexcept {
    auto roundedOpt = roundToFrame(time, rate);
    int64_t targetTicks = roundedOpt.has_value() ? roundedOpt->ticks() : time.ticks();
    int64_t clamped = std::clamp(targetTicks, int64_t{0}, duration.ticks());
    return TimelineTime(clamped);
}

} // namespace catchim::core
