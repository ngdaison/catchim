#include "Timecode.h"
#include <format>
#include <vector>
#include <sstream>

namespace catchim::core {

namespace {
constexpr int64_t SECONDS_PER_HOUR = 3600;
constexpr int64_t SECONDS_PER_MINUTE = 60;
constexpr int64_t CENTISECONDS_PER_SECOND = 100;
constexpr int64_t TICKS_PER_CENTISECOND = TICKS_PER_SECOND / CENTISECONDS_PER_SECOND;

int64_t getTicksPerFrame(const FrameRate& rate) {
    if (rate.numerator <= 0 || rate.denominator <= 0) return 4000;
    return (TICKS_PER_SECOND * static_cast<int64_t>(rate.denominator)) / rate.numerator;
}
} // namespace

std::string Timecode::format(TimelineTime time, TimecodeFormat format, const FrameRate& rate) {
    int64_t totalTicks = std::max<int64_t>(0, time.ticks());
    int64_t totalSeconds = totalTicks / TICKS_PER_SECOND;

    int64_t hourTicks = SECONDS_PER_HOUR * TICKS_PER_SECOND;
    int64_t minuteTicks = SECONDS_PER_MINUTE * TICKS_PER_SECOND;

    int64_t hours = totalTicks / hourTicks;
    int64_t minutes = (totalTicks % hourTicks) / minuteTicks;
    int64_t seconds = totalSeconds % SECONDS_PER_MINUTE;
    int64_t secondTicks = totalTicks % TICKS_PER_SECOND;

    switch (format) {
        case TimecodeFormat::MM_SS:
            return std::format("{:02}:{:02}", minutes, seconds);
        case TimecodeFormat::HH_MM_SS:
            return std::format("{:02}:{:02}:{:02}", hours, minutes, seconds);
        case TimecodeFormat::HH_MM_SS_CS: {
            int64_t centiseconds = secondTicks / TICKS_PER_CENTISECOND;
            return std::format("{:02}:{:02}:{:02}:{:02}", hours, minutes, seconds, centiseconds);
        }
        case TimecodeFormat::HH_MM_SS_FF: {
            int64_t tpf = getTicksPerFrame(rate);
            int64_t frames = (tpf > 0) ? (secondTicks / tpf) : 0;
            return std::format("{:02}:{:02}:{:02}:{:02}", hours, minutes, seconds, frames);
        }
    }
    return "";
}

std::optional<TimelineTime> Timecode::parse(
    std::string_view timecodeStr,
    TimecodeFormat format,
    const FrameRate& rate
) {
    if (timecodeStr.empty()) return std::nullopt;

    std::vector<int64_t> parts;
    size_t start = 0;
    while (start < timecodeStr.size()) {
        size_t end = timecodeStr.find(':', start);
        if (end == std::string_view::npos) end = timecodeStr.size();
        std::string_view part = timecodeStr.substr(start, end - start);
        try {
            parts.push_back(std::stoll(std::string(part)));
        } catch (...) {
            return std::nullopt;
        }
        start = end + 1;
    }

    switch (format) {
        case TimecodeFormat::MM_SS: {
            if (parts.size() != 2) return std::nullopt;
            int64_t m = parts[0];
            int64_t s = parts[1];
            if (s >= 60) return std::nullopt;
            return TimelineTime((m * 60 + s) * TICKS_PER_SECOND);
        }
        case TimecodeFormat::HH_MM_SS: {
            if (parts.size() != 3) return std::nullopt;
            int64_t h = parts[0];
            int64_t m = parts[1];
            int64_t s = parts[2];
            if (m >= 60 || s >= 60) return std::nullopt;
            return TimelineTime((h * 3600 + m * 60 + s) * TICKS_PER_SECOND);
        }
        case TimecodeFormat::HH_MM_SS_CS: {
            if (parts.size() != 4) return std::nullopt;
            int64_t h = parts[0];
            int64_t m = parts[1];
            int64_t s = parts[2];
            int64_t cs = parts[3];
            if (m >= 60 || s >= 60 || cs >= 100) return std::nullopt;
            return TimelineTime((h * 3600 + m * 60 + s) * TICKS_PER_SECOND + cs * TICKS_PER_CENTISECOND);
        }
        case TimecodeFormat::HH_MM_SS_FF: {
            if (parts.size() != 4) return std::nullopt;
            int64_t h = parts[0];
            int64_t m = parts[1];
            int64_t s = parts[2];
            int64_t f = parts[3];
            int64_t tpf = getTicksPerFrame(rate);
            int64_t bound = (rate.denominator > 0) ? (rate.numerator / rate.denominator) : 30;
            if (m >= 60 || s >= 60 || f >= bound) return std::nullopt;
            return TimelineTime((h * 3600 + m * 60 + s) * TICKS_PER_SECOND + f * tpf);
        }
    }
    return std::nullopt;
}

std::optional<TimecodeFormat> Timecode::guessFormat(std::string_view timecodeStr) {
    if (timecodeStr.empty()) return std::nullopt;
    size_t colons = 0;
    for (char c : timecodeStr) {
        if (c == ':') colons++;
    }
    if (colons == 1) return TimecodeFormat::MM_SS;
    if (colons == 2) return TimecodeFormat::HH_MM_SS;
    if (colons == 3) return TimecodeFormat::HH_MM_SS_FF;
    return std::nullopt;
}

} // namespace catchim::core
