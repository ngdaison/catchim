#include "RetimeResolutionEngine.h"

namespace catchim::editor {

double RetimeResolutionEngine::clampRetimeRate(double rate) noexcept {
    if (!std::isfinite(rate) || rate <= 0.0) {
        return DEFAULT_RETIME_RATE;
    }
    return std::clamp(rate, MIN_RETIME_RATE, MAX_RETIME_RATE);
}

bool RetimeResolutionEngine::canMaintainPitch(double rate) noexcept {
    return std::isfinite(rate) && rate > 0.0;
}

bool RetimeResolutionEngine::shouldMaintainPitch(double rate, bool maintainPitch) noexcept {
    return maintainPitch && canMaintainPitch(rate);
}

core::TimelineTime RetimeResolutionEngine::getSourceTimeAtClipTime(
    core::TimelineTime clipTime,
    double rate
) noexcept {
    const double safeRate = clampRetimeRate(rate);
    const int64_t sourceTicks = static_cast<int64_t>(std::round(static_cast<double>(clipTime.ticks()) * safeRate));
    return core::TimelineTime(sourceTicks);
}

core::TimelineTime RetimeResolutionEngine::getClipTimeAtSourceTime(
    core::TimelineTime sourceTime,
    double rate
) noexcept {
    const double safeRate = clampRetimeRate(rate);
    const int64_t clipTicks = static_cast<int64_t>(std::round(static_cast<double>(sourceTime.ticks()) / safeRate));
    return core::TimelineTime(clipTicks);
}

double RetimeResolutionEngine::getEffectiveRateAt(double rate) noexcept {
    return clampRetimeRate(rate);
}

core::TimelineTime RetimeResolutionEngine::getTimelineDurationForSourceSpan(
    core::TimelineTime sourceSpan,
    double rate
) noexcept {
    if (sourceSpan.ticks() <= 0) {
        return core::TimelineTime(0);
    }
    const double safeRate = clampRetimeRate(rate);
    const int64_t durationTicks = static_cast<int64_t>(std::round(static_cast<double>(sourceSpan.ticks()) / safeRate));
    return core::TimelineTime(durationTicks);
}

core::TimelineTime RetimeResolutionEngine::getSourceSpanAtClipTime(
    core::TimelineTime clipTime,
    double rate
) noexcept {
    const auto sourceTime = getSourceTimeAtClipTime(clipTime, rate);
    if (sourceTime.ticks() < 0) {
        return core::TimelineTime(0);
    }
    return sourceTime;
}

} // namespace catchim::editor
