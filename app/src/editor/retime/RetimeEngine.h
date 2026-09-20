#pragma once

#include "core/time/TimelineTime.h"
#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include <string>
#include <vector>
#include <memory>
#include <cmath>

namespace catchim::editor {

constexpr double DEFAULT_RETIME_RATE = 1.0;
constexpr double MIN_RETIME_RATE = 0.01;
constexpr double MAX_RETIME_RATE = 5.0;

struct RetimeConfig {
    double rate = DEFAULT_RETIME_RATE;
    bool maintainPitch = true;

    bool operator==(const RetimeConfig& other) const = default;
};

class RetimeEngine {
public:
    static double clampRate(double rate) noexcept;
    static bool canMaintainPitch(double rate) noexcept;
    static bool shouldMaintainPitch(double rate, bool maintainPitch) noexcept;

    // Time conversions
    static core::TimelineTime getSourceTimeAtClipTime(
        core::TimelineTime clipTime,
        double rate
    ) noexcept;

    static core::TimelineTime getClipTimeAtSourceTime(
        core::TimelineTime sourceTime,
        double rate
    ) noexcept;

    static core::TimelineTime getTimelineDurationForSourceSpan(
        core::TimelineTime sourceSpan,
        double rate
    ) noexcept;

    // Presets
    static const std::vector<double>& getStandardPresets() noexcept;
};

class ChangeClipSpeedCommand : public EditorCommand {
public:
    ChangeClipSpeedCommand(
        Timeline& timeline,
        const std::string& clipId,
        double newRate,
        bool adjustDuration = true
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Change Clip Speed"; }

private:
    Timeline& timeline_;
    std::string clipId_;
    double newRate_;
    bool adjustDuration_;

    // Stored state for undo
    double oldRate_ = DEFAULT_RETIME_RATE;
    core::TimelineTime oldDuration_ = core::TimelineTime(0);
    bool executed_ = false;
};

} // namespace catchim::editor
