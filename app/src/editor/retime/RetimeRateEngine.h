#pragma once

#include "core/time/TimelineTime.h"
#include "editor/retime/RetimeEngine.h"
#include <utility>

namespace catchim::editor {

class RetimeRateEngine {
public:
    static constexpr double DEFAULT_RETIME_RATE = 1.0;
    static constexpr double MIN_RETIME_RATE = 0.01;
    static constexpr double MAX_RETIME_RATE = 5.0;

    static double clampRetimeRate(double rate) noexcept;
    static bool canMaintainPitch(double rate) noexcept;
    static bool shouldMaintainPitch(double rate, bool maintainPitch) noexcept;

    static RetimeConfig buildConstantRetime(double rate, bool maintainPitch = false) noexcept;

    static double getSourceTimeAtClipTime(double clipTimeSeconds, double rate) noexcept;
    static double getClipTimeAtSourceTime(double sourceTimeSeconds, double rate) noexcept;
    static double getTimelineDurationForSourceSpan(double sourceSpanSeconds, double rate) noexcept;
    static double getSourceSpanAtClipTime(double clipTimeSeconds, double rate) noexcept;

    static core::TimelineTime getSourceTimeAtClipTime(core::TimelineTime clipTime, double rate) noexcept;
    static core::TimelineTime getClipTimeAtSourceTime(core::TimelineTime sourceTime, double rate) noexcept;
    static core::TimelineTime getTimelineDurationForSourceSpan(core::TimelineTime sourceSpan, double rate) noexcept;

    static std::pair<RetimeConfig, RetimeConfig> splitRetimeAtClipTime(
        const RetimeConfig& config,
        double splitClipTimeSeconds
    ) noexcept;
};

} // namespace catchim::editor
