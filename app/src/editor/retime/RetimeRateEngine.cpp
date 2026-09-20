#include "RetimeRateEngine.h"
#include <cmath>
#include <algorithm>

namespace catchim::editor {

double RetimeRateEngine::clampRetimeRate(double rate) noexcept {
    if (!std::isfinite(rate) || rate <= 0.0) {
        return DEFAULT_RETIME_RATE;
    }
    return std::clamp(rate, MIN_RETIME_RATE, MAX_RETIME_RATE);
}

bool RetimeRateEngine::canMaintainPitch(double rate) noexcept {
    return std::isfinite(rate) && rate > 0.0;
}

bool RetimeRateEngine::shouldMaintainPitch(double rate, bool maintainPitch) noexcept {
    return maintainPitch && canMaintainPitch(rate);
}

RetimeConfig RetimeRateEngine::buildConstantRetime(double rate, bool maintainPitch) noexcept {
    return RetimeConfig{
        .rate = clampRetimeRate(rate),
        .maintainPitch = maintainPitch
    };
}

double RetimeRateEngine::getSourceTimeAtClipTime(double clipTimeSeconds, double rate) noexcept {
    return clipTimeSeconds * clampRetimeRate(rate);
}

double RetimeRateEngine::getClipTimeAtSourceTime(double sourceTimeSeconds, double rate) noexcept {
    return sourceTimeSeconds / clampRetimeRate(rate);
}

double RetimeRateEngine::getTimelineDurationForSourceSpan(double sourceSpanSeconds, double rate) noexcept {
    if (sourceSpanSeconds <= 0.0) return 0.0;
    return sourceSpanSeconds / clampRetimeRate(rate);
}

double RetimeRateEngine::getSourceSpanAtClipTime(double clipTimeSeconds, double rate) noexcept {
    return std::max(0.0, getSourceTimeAtClipTime(clipTimeSeconds, rate));
}

core::TimelineTime RetimeRateEngine::getSourceTimeAtClipTime(core::TimelineTime clipTime, double rate) noexcept {
    return core::TimelineTime::fromSeconds(getSourceTimeAtClipTime(clipTime.toSeconds(), rate));
}

core::TimelineTime RetimeRateEngine::getClipTimeAtSourceTime(core::TimelineTime sourceTime, double rate) noexcept {
    return core::TimelineTime::fromSeconds(getClipTimeAtSourceTime(sourceTime.toSeconds(), rate));
}

core::TimelineTime RetimeRateEngine::getTimelineDurationForSourceSpan(core::TimelineTime sourceSpan, double rate) noexcept {
    return core::TimelineTime::fromSeconds(getTimelineDurationForSourceSpan(sourceSpan.toSeconds(), rate));
}

std::pair<RetimeConfig, RetimeConfig> RetimeRateEngine::splitRetimeAtClipTime(
    const RetimeConfig& config,
    double /*splitClipTimeSeconds*/
) noexcept {
    return {config, config};
}

} // namespace catchim::editor
