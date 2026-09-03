#include "opencut/time.hpp"

#include <charconv>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <vector>

namespace opencut {

namespace {

std::int64_t div_euclid(std::int64_t a, std::int64_t b) noexcept {
    auto q = a / b;
    auto r = a % b;
    if (r < 0) {
        q += (b > 0) ? -1 : 1;
    }
    return q;
}

std::int64_t rem_euclid(std::int64_t a, std::int64_t b) noexcept {
    auto r = a % b;
    if (r < 0) {
        r += (b > 0) ? b : -b;
    }
    return r;
}

std::string_view trim(std::string_view sv) noexcept {
    while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t' || sv.front() == '\r' || sv.front() == '\n')) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && (sv.back() == ' ' || sv.back() == '\t' || sv.back() == '\r' || sv.back() == '\n')) {
        sv.remove_suffix(1);
    }
    return sv;
}

std::vector<std::string_view> split(std::string_view sv, char delim) {
    std::vector<std::string_view> tokens;
    std::size_t start = 0;
    while (start < sv.size()) {
        auto pos = sv.find(delim, start);
        if (pos == std::string_view::npos) {
            tokens.push_back(sv.substr(start));
            break;
        }
        tokens.push_back(sv.substr(start, pos - start));
        start = pos + 1;
    }
    return tokens;
}

std::optional<std::uint32_t> parse_uint(std::string_view sv) noexcept {
    if (sv.empty()) return std::nullopt;
    std::uint32_t val = 0;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
    if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
        return std::nullopt;
    }
    return val;
}

} // namespace

std::optional<MediaTime> MediaTime::from_seconds_f64(double seconds) noexcept {
    if (!std::isfinite(seconds)) return std::nullopt;
    auto ticks = static_cast<std::int64_t>(std::round(seconds * TICKS_PER_SECOND_F64));
    return MediaTime(ticks);
}

double MediaTime::to_seconds_f64() const noexcept {
    return static_cast<double>(m_ticks) / TICKS_PER_SECOND_F64;
}

std::optional<MediaTime> MediaTime::from_frame(std::int64_t frame, FrameRate rate) noexcept {
    auto tpf = rate.ticks_per_frame();
    if (!tpf) return std::nullopt;
    return MediaTime(frame * (*tpf));
}

std::optional<std::int64_t> MediaTime::to_frame_round(FrameRate rate) const noexcept {
    auto tpf_opt = rate.ticks_per_frame();
    if (!tpf_opt) return std::nullopt;
    auto tpf = *tpf_opt;
    auto remainder = rem_euclid(m_ticks, tpf);
    auto floor = div_euclid(m_ticks, tpf);
    if (remainder * 2 >= tpf) {
        return floor + 1;
    }
    return floor;
}

std::optional<std::int64_t> MediaTime::to_frame_floor(FrameRate rate) const noexcept {
    auto tpf = rate.ticks_per_frame();
    if (!tpf) return std::nullopt;
    return div_euclid(m_ticks, *tpf);
}

std::optional<MediaTime> MediaTime::round_to_frame(FrameRate rate) const noexcept {
    auto frame = to_frame_round(rate);
    if (!frame) return std::nullopt;
    return from_frame(*frame, rate);
}

std::optional<MediaTime> MediaTime::floor_to_frame(FrameRate rate) const noexcept {
    auto tpf = rate.ticks_per_frame();
    if (!tpf) return std::nullopt;
    auto floor = div_euclid(m_ticks, *tpf);
    return MediaTime(floor * (*tpf));
}

std::optional<bool> MediaTime::is_frame_aligned(FrameRate rate) const noexcept {
    auto tpf = rate.ticks_per_frame();
    if (!tpf) return std::nullopt;
    return rem_euclid(m_ticks, *tpf) == 0;
}

std::optional<MediaTime> MediaTime::last_frame_time(FrameRate rate) const noexcept {
    if (m_ticks <= 0) {
        return MediaTime::zero();
    }
    auto last_tick = m_ticks - 1;
    return MediaTime(last_tick).floor_to_frame(rate);
}

std::optional<MediaTime> MediaTime::snapped_seek_time(MediaTime duration, FrameRate rate) const noexcept {
    auto snapped = round_to_frame(rate);
    if (!snapped) return std::nullopt;
    return snapped->clamp(MediaTime::zero(), duration);
}

std::optional<TimeCodeFormat> guess_timecode_format(std::string_view time_code) noexcept {
    auto trimmed = trim(time_code);
    if (trimmed.empty()) return std::nullopt;

    auto parts = split(trimmed, ':');
    if (parts.empty()) return std::nullopt;

    for (auto part : parts) {
        if (!parse_uint(part)) return std::nullopt;
    }

    switch (parts.size()) {
        case 2: return TimeCodeFormat::MmSs;
        case 3: return TimeCodeFormat::HhMmSs;
        case 4: return TimeCodeFormat::HhMmSsFf;
        default: return std::nullopt;
    }
}

std::optional<std::string> format_timecode(
    MediaTime time,
    std::optional<TimeCodeFormat> format,
    std::optional<FrameRate> rate) noexcept {
    auto fmt = format.value_or(TimeCodeFormat::HhMmSsCs);
    auto total_ticks = time.as_ticks() < 0 ? 0 : time.as_ticks();

    auto total_seconds = total_ticks / TICKS_PER_SECOND;
    auto hour_ticks = SECONDS_PER_HOUR * TICKS_PER_SECOND;
    auto minute_ticks = SECONDS_PER_MINUTE * TICKS_PER_SECOND;

    auto hours = total_ticks / hour_ticks;
    auto minutes = (total_ticks % hour_ticks) / minute_ticks;
    auto seconds = total_seconds % SECONDS_PER_MINUTE;
    auto second_ticks = total_ticks % TICKS_PER_SECOND;
    auto centiseconds = second_ticks / TICKS_PER_CENTISECOND;

    char buffer[64];
    switch (fmt) {
        case TimeCodeFormat::MmSs:
            std::snprintf(buffer, sizeof(buffer), "%02lld:%02lld",
                          static_cast<long long>(minutes),
                          static_cast<long long>(seconds));
            return std::string(buffer);

        case TimeCodeFormat::HhMmSs:
            std::snprintf(buffer, sizeof(buffer), "%02lld:%02lld:%02lld",
                          static_cast<long long>(hours),
                          static_cast<long long>(minutes),
                          static_cast<long long>(seconds));
            return std::string(buffer);

        case TimeCodeFormat::HhMmSsCs:
            std::snprintf(buffer, sizeof(buffer), "%02lld:%02lld:%02lld:%02lld",
                          static_cast<long long>(hours),
                          static_cast<long long>(minutes),
                          static_cast<long long>(seconds),
                          static_cast<long long>(centiseconds));
            return std::string(buffer);

        case TimeCodeFormat::HhMmSsFf: {
            if (!rate) return std::nullopt;
            auto tpf = rate->ticks_per_frame();
            if (!tpf) return std::nullopt;
            auto frames = second_ticks / (*tpf);
            std::snprintf(buffer, sizeof(buffer), "%02lld:%02lld:%02lld:%02lld",
                          static_cast<long long>(hours),
                          static_cast<long long>(minutes),
                          static_cast<long long>(seconds),
                          static_cast<long long>(frames));
            return std::string(buffer);
        }
    }
    return std::nullopt;
}

std::optional<MediaTime> parse_timecode(
    std::string_view time_code,
    std::optional<TimeCodeFormat> format,
    std::optional<FrameRate> rate) noexcept {
    auto trimmed = trim(time_code);
    if (trimmed.empty()) return std::nullopt;

    auto fmt = format.value_or(TimeCodeFormat::HhMmSsCs);
    auto parts = split(trimmed, ':');

    std::vector<std::uint32_t> parsed;
    parsed.reserve(parts.size());
    for (auto part : parts) {
        auto val = parse_uint(part);
        if (!val) return std::nullopt;
        parsed.push_back(*val);
    }

    switch (fmt) {
        case TimeCodeFormat::MmSs: {
            if (parsed.size() != 2) return std::nullopt;
            auto minutes = parsed[0];
            auto seconds = parsed[1];
            if (seconds >= SECONDS_PER_MINUTE) return std::nullopt;
            return MediaTime::from_ticks(
                (static_cast<std::int64_t>(minutes) * SECONDS_PER_MINUTE + static_cast<std::int64_t>(seconds)) * TICKS_PER_SECOND
            );
        }

        case TimeCodeFormat::HhMmSs: {
            if (parsed.size() != 3) return std::nullopt;
            auto hours = parsed[0];
            auto minutes = parsed[1];
            auto seconds = parsed[2];
            if (minutes >= SECONDS_PER_MINUTE || seconds >= SECONDS_PER_MINUTE) return std::nullopt;
            return MediaTime::from_ticks(
                (static_cast<std::int64_t>(hours) * SECONDS_PER_HOUR +
                 static_cast<std::int64_t>(minutes) * SECONDS_PER_MINUTE +
                 static_cast<std::int64_t>(seconds)) * TICKS_PER_SECOND
            );
        }

        case TimeCodeFormat::HhMmSsCs: {
            if (parsed.size() != 4) return std::nullopt;
            auto hours = parsed[0];
            auto minutes = parsed[1];
            auto seconds = parsed[2];
            auto centiseconds = parsed[3];
            if (minutes >= SECONDS_PER_MINUTE || seconds >= SECONDS_PER_MINUTE || centiseconds >= CENTISECONDS_PER_SECOND) {
                return std::nullopt;
            }
            return MediaTime::from_ticks(
                (static_cast<std::int64_t>(hours) * SECONDS_PER_HOUR +
                 static_cast<std::int64_t>(minutes) * SECONDS_PER_MINUTE +
                 static_cast<std::int64_t>(seconds)) * TICKS_PER_SECOND +
                 static_cast<std::int64_t>(centiseconds) * TICKS_PER_CENTISECOND
            );
        }

        case TimeCodeFormat::HhMmSsFf: {
            if (!rate) return std::nullopt;
            auto bound = rate->frame_number_upper_bound();
            auto tpf = rate->ticks_per_frame();
            if (!bound || !tpf || parsed.size() != 4) return std::nullopt;

            auto hours = parsed[0];
            auto minutes = parsed[1];
            auto seconds = parsed[2];
            auto frames = parsed[3];
            if (minutes >= SECONDS_PER_MINUTE || seconds >= SECONDS_PER_MINUTE || frames >= *bound) {
                return std::nullopt;
            }
            return MediaTime::from_ticks(
                (static_cast<std::int64_t>(hours) * SECONDS_PER_HOUR +
                 static_cast<std::int64_t>(minutes) * SECONDS_PER_MINUTE +
                 static_cast<std::int64_t>(seconds)) * TICKS_PER_SECOND +
                 static_cast<std::int64_t>(frames) * (*tpf)
            );
        }
    }

    return std::nullopt;
}

} // namespace opencut
