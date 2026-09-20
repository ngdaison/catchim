#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <compare>

namespace catchim::core {

constexpr int64_t TICKS_PER_SECOND = 120'000;
constexpr double TICKS_PER_SECOND_F64 = 120'000.0;

class TimelineTime;

struct FrameRate {
    int32_t numerator{30};
    int32_t denominator{1};

    constexpr double toFps() const noexcept {
        return denominator > 0 ? static_cast<double>(numerator) / denominator : 30.0;
    }

    TimelineTime frameDuration() const noexcept;

    constexpr bool operator==(const FrameRate& other) const noexcept = default;
};

class TimelineTime {
public:
    constexpr TimelineTime() noexcept : ticks_(0) {}
    constexpr explicit TimelineTime(int64_t ticks) noexcept : ticks_(ticks) {}

    static constexpr TimelineTime fromTicks(int64_t ticks) noexcept {
        return TimelineTime(ticks);
    }

    static TimelineTime fromSeconds(double seconds) noexcept {
        if (!std::isfinite(seconds)) return TimelineTime(0);
        return TimelineTime(static_cast<int64_t>(std::round(seconds * TICKS_PER_SECOND_F64)));
    }

    static constexpr TimelineTime zero() noexcept {
        return TimelineTime(0);
    }

    constexpr int64_t ticks() const noexcept { return ticks_; }

    double toSeconds() const noexcept {
        return static_cast<double>(ticks_) / TICKS_PER_SECOND_F64;
    }

    double toMilliseconds() const noexcept {
        return toSeconds() * 1000.0;
    }

    int64_t toMicroseconds() const noexcept {
        return static_cast<int64_t>(toSeconds() * 1'000'000.0);
    }

    // Frame helpers
    TimelineTime roundToFrame(const FrameRate& rate) const noexcept;
    TimelineTime floorToFrame(const FrameRate& rate) const noexcept;
    static TimelineTime lastFrameTime(TimelineTime duration, const FrameRate& rate) noexcept;
    TimelineTime snappedSeek(TimelineTime duration, const FrameRate& rate) const noexcept;

    // Operators
    constexpr TimelineTime operator+(const TimelineTime& other) const noexcept {
        return TimelineTime(ticks_ + other.ticks_);
    }

    constexpr TimelineTime operator-(const TimelineTime& other) const noexcept {
        return TimelineTime(ticks_ - other.ticks_);
    }

    constexpr TimelineTime operator*(double factor) const noexcept {
        return TimelineTime(static_cast<int64_t>(std::round(ticks_ * factor)));
    }

    constexpr TimelineTime operator/(double divisor) const noexcept {
        return TimelineTime(static_cast<int64_t>(std::round(ticks_ / divisor)));
    }

    constexpr TimelineTime& operator+=(const TimelineTime& other) noexcept {
        ticks_ += other.ticks_;
        return *this;
    }

    constexpr TimelineTime& operator-=(const TimelineTime& other) noexcept {
        ticks_ -= other.ticks_;
        return *this;
    }

    constexpr auto operator<=>(const TimelineTime& other) const noexcept = default;

    constexpr TimelineTime clamp(TimelineTime minVal, TimelineTime maxVal) const noexcept {
        return TimelineTime(std::clamp(ticks_, minVal.ticks_, maxVal.ticks_));
    }

private:
    int64_t ticks_{0};
};

inline TimelineTime FrameRate::frameDuration() const noexcept {
    int64_t den = denominator > 0 ? static_cast<int64_t>(denominator) : 1;
    int64_t num = numerator > 0 ? static_cast<int64_t>(numerator) : 30;
    return TimelineTime((TICKS_PER_SECOND * den) / num);
}

} // namespace catchim::core
