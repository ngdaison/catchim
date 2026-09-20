#pragma once

#include "core/time/TimelineTime.h"
#include <string>
#include <vector>

namespace catchim::editor {

struct RulerConfig {
    double labelIntervalSeconds{1.0};
    double tickIntervalSeconds{0.2};
};

struct RulerTick {
    core::TimelineTime time;
    double pixelX{0.0};
    bool isMajor{false};
    std::string label;
};

class RulerEngine {
public:
    static constexpr double BASE_PIXELS_PER_SECOND = 100.0;
    static constexpr double MIN_LABEL_SPACING_PX = 120.0;
    static constexpr double MIN_TICK_SPACING_PX = 18.0;

    static RulerConfig getRulerConfig(double zoomLevel, double fps);
    static bool shouldShowLabel(double timeInSeconds, double labelIntervalSeconds);
    static std::string formatRulerLabel(double timeInSeconds, double fps);

    static std::vector<RulerTick> generateRulerTicks(
        core::TimelineTime startTime,
        core::TimelineTime endTime,
        double zoomLevel,
        double fps,
        double scrollOffsetX = 0.0
    );
};

} // namespace catchim::editor
