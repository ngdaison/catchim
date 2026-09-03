#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace opencut {

inline constexpr std::int64_t TICKS_PER_SECOND = 120'000;
inline constexpr double TICKS_PER_SECOND_F64 = 120'000.0;
inline constexpr std::int64_t SECONDS_PER_HOUR = 3'600;
inline constexpr std::int64_t SECONDS_PER_MINUTE = 60;
inline constexpr std::int64_t CENTISECONDS_PER_SECOND = 100;
inline constexpr std::int64_t TICKS_PER_CENTISECOND = TICKS_PER_SECOND / CENTISECONDS_PER_SECOND;

struct FrameRate {
    std::uint32_t numerator = 30;
    std::uint32_t denominator = 1;

    static constexpr FrameRate fps_23_976() noexcept { return {24'000, 1'001}; }
    static constexpr FrameRate fps_24() noexcept { return {24, 1}; }
    static constexpr FrameRate fps_25() noexcept { return {25, 1}; }
    static constexpr FrameRate fps_29_97() noexcept { return {30'000, 1'001}; }
    static constexpr FrameRate fps_30() noexcept { return {30, 1}; }
    static constexpr FrameRate fps_48() noexcept { return {48, 1}; }
    static constexpr FrameRate fps_50() noexcept { return {50, 1}; }
    static constexpr FrameRate fps_59_94() noexcept { return {60'000, 1'001}; }
    static constexpr FrameRate fps_60() noexcept { return {60, 1}; }
    static constexpr FrameRate fps_120() noexcept { return {120, 1}; }

    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return numerator > 0 && denominator > 0;
    }

    [[nodiscard]] std::optional<double> as_f64() const noexcept {
        if (!is_valid()) return std::nullopt;
        return static_cast<double>(numerator) / static_cast<double>(denominator);
    }

    [[nodiscard]] std::optional<std::uint32_t> frame_number_upper_bound() const noexcept {
        if (!is_valid()) return std::nullopt;
        return (numerator + denominator - 1) / denominator;
    }

    [[nodiscard]] std::optional<std::int64_t> ticks_per_frame() const noexcept {
        if (!is_valid()) return std::nullopt;
        auto tick_num = TICKS_PER_SECOND * static_cast<std::int64_t>(denominator);
        auto tick_den = static_cast<std::int64_t>(numerator);
        if (tick_den == 0 || tick_num % tick_den != 0) return std::nullopt;
        return tick_num / tick_den;
    }

    constexpr bool operator==(const FrameRate& other) const noexcept = default;
};

class MediaTime {
public:
    constexpr MediaTime() noexcept : m_ticks(0) {}
    constexpr explicit MediaTime(std::int64_t ticks) noexcept : m_ticks(ticks) {}

    static constexpr MediaTime zero() noexcept { return MediaTime(0); }
    static constexpr MediaTime one_tick() noexcept { return MediaTime(1); }

    [[nodiscard]] static constexpr MediaTime from_ticks(std::int64_t ticks) noexcept {
        return MediaTime(ticks);
    }

    [[nodiscard]] constexpr std::int64_t as_ticks() const noexcept { return m_ticks; }

    [[nodiscard]] static std::optional<MediaTime> from_seconds_f64(double seconds) noexcept;
    [[nodiscard]] double to_seconds_f64() const noexcept;

    [[nodiscard]] static std::optional<MediaTime> from_frame(std::int64_t frame, FrameRate rate) noexcept;
    [[nodiscard]] std::optional<std::int64_t> to_frame_round(FrameRate rate) const noexcept;
    [[nodiscard]] std::optional<std::int64_t> to_frame_floor(FrameRate rate) const noexcept;

    [[nodiscard]] std::optional<MediaTime> round_to_frame(FrameRate rate) const noexcept;
    [[nodiscard]] std::optional<MediaTime> floor_to_frame(FrameRate rate) const noexcept;
    [[nodiscard]] std::optional<bool> is_frame_aligned(FrameRate rate) const noexcept;
    [[nodiscard]] std::optional<MediaTime> last_frame_time(FrameRate rate) const noexcept;
    [[nodiscard]] std::optional<MediaTime> snapped_seek_time(MediaTime duration, FrameRate rate) const noexcept;

    [[nodiscard]] constexpr MediaTime clamp(MediaTime min_val, MediaTime max_val) const noexcept {
        if (m_ticks < min_val.m_ticks) return min_val;
        if (m_ticks > max_val.m_ticks) return max_val;
        return *this;
    }

    [[nodiscard]] constexpr MediaTime min(MediaTime other) const noexcept {
        return m_ticks < other.m_ticks ? *this : other;
    }

    [[nodiscard]] constexpr MediaTime max(MediaTime other) const noexcept {
        return m_ticks > other.m_ticks ? *this : other;
    }

    constexpr MediaTime operator+(MediaTime other) const noexcept {
        return MediaTime(m_ticks + other.m_ticks);
    }

    constexpr MediaTime operator-(MediaTime other) const noexcept {
        return MediaTime(m_ticks - other.m_ticks);
    }

    constexpr auto operator<=>(const MediaTime&) const noexcept = default;

private:
    std::int64_t m_ticks = 0;
};

enum class TimeCodeFormat {
    MmSs,
    HhMmSs,
    HhMmSsCs,
    HhMmSsFf,
};

[[nodiscard]] std::optional<TimeCodeFormat> guess_timecode_format(std::string_view time_code) noexcept;

[[nodiscard]] std::optional<std::string> format_timecode(
    MediaTime time,
    std::optional<TimeCodeFormat> format = std::nullopt,
    std::optional<FrameRate> rate = std::nullopt) noexcept;

[[nodiscard]] std::optional<MediaTime> parse_timecode(
    std::string_view time_code,
    std::optional<TimeCodeFormat> format = std::nullopt,
    std::optional<FrameRate> rate = std::nullopt) noexcept;

} // namespace opencut
