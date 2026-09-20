#pragma once

#include "core/time/TimelineTime.h"
#include <vector>
#include <string>
#include <optional>

namespace catchim::core {

struct StandardFrameRate {
    double value;
    FrameRate rate;
    const char* label;
};

class RationalFrameRateHelper {
public:
    static const std::vector<StandardFrameRate>& standardRates() noexcept;

    static double toFloat(const FrameRate& rate) noexcept;
    static bool areEqual(const FrameRate& a, const FrameRate& b) noexcept;
    static FrameRate fromFloat(double fps, double tolerance = 0.01) noexcept;

    static int64_t gcd(int64_t a, int64_t b) noexcept;

    static int64_t roundFrameTicks(int64_t ticks, const FrameRate& fps) noexcept;
    static int64_t floorFrameTicks(int64_t ticks, const FrameRate& fps) noexcept;
    static int64_t lastFrameTicks(int64_t durationTicks, const FrameRate& fps) noexcept;

    static std::optional<double> getHighestImportedVideoFps(
        const std::vector<double>& videoFpsList
    ) noexcept;

    static std::optional<FrameRate> getRaisedProjectFpsForImportedMedia(
        const FrameRate& currentFps,
        const std::vector<double>& importedVideoFpsList
    ) noexcept;
};

} // namespace catchim::core
