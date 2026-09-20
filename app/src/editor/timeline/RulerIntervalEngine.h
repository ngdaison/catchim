#pragma once

#include "core/time/RationalFrameRate.h"
#include "editor/timeline/RulerEngine.h"
#include <array>
#include <string>
#include <cstdint>

namespace catchim::editor {

class RulerIntervalEngine {
public:
    static constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
    static constexpr double MIN_LABEL_SPACING_PX = 120.0;
    static constexpr double MIN_TICK_SPACING_PX = 18.0;

    static constexpr std::array<int, 5> LABEL_FRAME_INTERVALS = {2, 3, 5, 10, 15};
    static constexpr std::array<int, 6> TICK_FRAME_INTERVALS = {1, 2, 3, 5, 10, 15};
    static constexpr std::array<int, 14> SECOND_MULTIPLIERS = {
        1, 2, 3, 5, 10, 15, 30, 60, 120, 300, 600, 900, 1800, 3600
    };

    static RulerConfig getRulerConfig(
        double zoomLevel,
        const core::FrameRate& fps
    ) noexcept;

    static RulerConfig getRulerConfig(
        double zoomLevel,
        double fpsFloat
    ) noexcept;

    static bool shouldShowLabel(
        double timeInSeconds,
        double labelIntervalSeconds
    ) noexcept;

    static std::string formatRulerLabel(
        double timeInSeconds,
        const core::FrameRate& fps
    );

    static std::string formatRulerLabel(
        double timeInSeconds,
        double fpsFloat
    );

    static bool isSecondBoundary(double timeInSeconds) noexcept;

    static int getFrameWithinSecond(
        double timeInSeconds,
        double fpsFloat
    ) noexcept;

    static std::string formatTimestamp(double timeInSeconds);

private:
    static double ensureTickDividesLabel(
        double tickIntervalSeconds,
        double labelIntervalSeconds,
        double pixelsPerFrame,
        double pixelsPerSecond,
        double fpsFloat
    ) noexcept;

    static double findOptimalInterval(
        double pixelsPerFrame,
        double pixelsPerSecond,
        double fpsFloat,
        double minSpacingPx,
        const int* frameIntervals,
        size_t frameIntervalsCount
    ) noexcept;
};

} // namespace catchim::editor
