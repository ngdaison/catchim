#pragma once

#include "core/time/TimelineTime.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

/**
 * @brief Retime speed rates and temporal resolution engine between clip and source time domains.
 * Corresponds to web/src/retime/ (rate.ts, resolve.ts, split.ts, presets.ts).
 */
class RetimeResolutionEngine {
public:
    static constexpr double DEFAULT_RETIME_RATE = 1.0;
    static constexpr double MIN_RETIME_RATE = 0.01;
    static constexpr double MAX_RETIME_RATE = 5.0;

    static constexpr double MIN_RATE = MIN_RETIME_RATE;
    static constexpr double MAX_RATE = MAX_RETIME_RATE;

    static double clampRetimeRate(double rate) noexcept;
    static bool canMaintainPitch(double rate) noexcept;
    static bool shouldMaintainPitch(double rate, bool maintainPitch = false) noexcept;

    static core::TimelineTime getSourceTimeAtClipTime(
        core::TimelineTime clipTime,
        double rate = DEFAULT_RETIME_RATE
    ) noexcept;

    static core::TimelineTime getClipTimeAtSourceTime(
        core::TimelineTime sourceTime,
        double rate = DEFAULT_RETIME_RATE
    ) noexcept;

    static double getEffectiveRateAt(double rate = DEFAULT_RETIME_RATE) noexcept;

    static core::TimelineTime getTimelineDurationForSourceSpan(
        core::TimelineTime sourceSpan,
        double rate = DEFAULT_RETIME_RATE
    ) noexcept;

    static core::TimelineTime getSourceSpanAtClipTime(
        core::TimelineTime clipTime,
        double rate = DEFAULT_RETIME_RATE
    ) noexcept;
};

} // namespace catchim::editor
