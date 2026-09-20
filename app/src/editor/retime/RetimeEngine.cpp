#include "editor/retime/RetimeEngine.h"
#include <algorithm>

namespace catchim::editor {

double RetimeEngine::clampRate(double rate) noexcept {
    if (!std::isfinite(rate) || rate <= 0.0) {
        return DEFAULT_RETIME_RATE;
    }
    return std::clamp(rate, MIN_RETIME_RATE, MAX_RETIME_RATE);
}

bool RetimeEngine::canMaintainPitch(double rate) noexcept {
    return std::isfinite(rate) && rate > 0.0;
}

bool RetimeEngine::shouldMaintainPitch(double rate, bool maintainPitch) noexcept {
    return maintainPitch && canMaintainPitch(rate);
}

core::TimelineTime RetimeEngine::getSourceTimeAtClipTime(
    core::TimelineTime clipTime,
    double rate
) noexcept {
    double safeRate = clampRate(rate);
    int64_t ticks = static_cast<int64_t>(std::round(clipTime.ticks() * safeRate));
    return core::TimelineTime(ticks);
}

core::TimelineTime RetimeEngine::getClipTimeAtSourceTime(
    core::TimelineTime sourceTime,
    double rate
) noexcept {
    double safeRate = clampRate(rate);
    int64_t ticks = static_cast<int64_t>(std::round(sourceTime.ticks() / safeRate));
    return core::TimelineTime(ticks);
}

core::TimelineTime RetimeEngine::getTimelineDurationForSourceSpan(
    core::TimelineTime sourceSpan,
    double rate
) noexcept {
    if (sourceSpan.ticks() <= 0) {
        return core::TimelineTime(0);
    }
    double safeRate = clampRate(rate);
    int64_t ticks = static_cast<int64_t>(std::round(sourceSpan.ticks() / safeRate));
    return core::TimelineTime(ticks);
}

const std::vector<double>& RetimeEngine::getStandardPresets() noexcept {
    static const std::vector<double> presets = {0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 4.0};
    return presets;
}

// ChangeClipSpeedCommand implementation
ChangeClipSpeedCommand::ChangeClipSpeedCommand(
    Timeline& timeline,
    const std::string& clipId,
    double newRate,
    bool adjustDuration
)
    : timeline_(timeline)
    , clipId_(clipId)
    , newRate_(RetimeEngine::clampRate(newRate))
    , adjustDuration_(adjustDuration)
{
}

bool ChangeClipSpeedCommand::execute() {
    Clip* clip = timeline_.findClip(core::ClipId(clipId_));
    if (!clip) return false;

    if (!executed_) {
        oldRate_ = clip->getParam<double>("retime_rate", DEFAULT_RETIME_RATE);
        oldDuration_ = clip->duration();
        executed_ = true;
    }

    clip->setParam("retime_rate", newRate_);

    if (adjustDuration_) {
        // Source span = duration * oldRate
        // New duration = sourceSpan / newRate
        core::TimelineTime sourceSpan = RetimeEngine::getSourceTimeAtClipTime(oldDuration_, oldRate_);
        core::TimelineTime newDuration = RetimeEngine::getTimelineDurationForSourceSpan(sourceSpan, newRate_);
        if (newDuration.ticks() > 0) {
            clip->setDuration(newDuration);
        }
    }

    return true;
}

bool ChangeClipSpeedCommand::undo() {
    Clip* clip = timeline_.findClip(core::ClipId(clipId_));
    if (!clip) return false;

    clip->setParam("retime_rate", oldRate_);
    if (adjustDuration_) {
        clip->setDuration(oldDuration_);
    }

    return true;
}

} // namespace catchim::editor
