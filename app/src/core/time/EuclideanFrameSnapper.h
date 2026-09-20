#pragma once

#include "core/time/TimelineTime.h"
#include "core/time/RationalFrameRate.h"
#include <optional>
#include <cstdint>

namespace catchim::core {

class EuclideanFrameSnapper {
public:
    static constexpr int64_t TICKS_PER_SECOND = 120'000;

    static int64_t divEuclid(int64_t a, int64_t b) noexcept;
    static int64_t remEuclid(int64_t a, int64_t b) noexcept;

    static std::optional<int64_t> getTicksPerFrame(const FrameRate& rate) noexcept;

    static std::optional<TimelineTime> roundToFrame(
        TimelineTime time,
        const FrameRate& rate
    ) noexcept;

    static std::optional<TimelineTime> floorToFrame(
        TimelineTime time,
        const FrameRate& rate
    ) noexcept;

    static TimelineTime lastFrameTime(
        TimelineTime duration,
        const FrameRate& rate
    ) noexcept;

    static TimelineTime snappedSeekTime(
        TimelineTime time,
        TimelineTime duration,
        const FrameRate& rate
    ) noexcept;
};

} // namespace catchim::core
