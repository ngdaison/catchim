#include "core/time/RationalFrameRate.h"
#include <cmath>
#include <algorithm>

namespace catchim::core {

const std::vector<StandardFrameRate>& RationalFrameRateHelper::standardRates() noexcept {
    static const std::vector<StandardFrameRate> s_rates = {
        {24000.0 / 1001.0, FrameRate{24000, 1001}, "23.976 fps"},
        {24.0,             FrameRate{24, 1},       "24 fps"},
        {25.0,             FrameRate{25, 1},       "25 fps"},
        {30000.0 / 1001.0, FrameRate{30000, 1001}, "29.97 fps"},
        {30.0,             FrameRate{30, 1},       "30 fps"},
        {48.0,             FrameRate{48, 1},       "48 fps"},
        {50.0,             FrameRate{50, 1},       "50 fps"},
        {60000.0 / 1001.0, FrameRate{60000, 1001}, "59.94 fps"},
        {60.0,             FrameRate{60, 1},       "60 fps"},
        {120.0,            FrameRate{120, 1},      "120 fps"}
    };
    return s_rates;
}

double RationalFrameRateHelper::toFloat(const FrameRate& rate) noexcept {
    return rate.toFps();
}

bool RationalFrameRateHelper::areEqual(const FrameRate& a, const FrameRate& b) noexcept {
    return a.numerator == b.numerator && a.denominator == b.denominator;
}

int64_t RationalFrameRateHelper::gcd(int64_t a, int64_t b) noexcept {
    int64_t x = std::abs(a);
    int64_t y = std::abs(b);
    while (y != 0) {
        int64_t rem = x % y;
        x = y;
        y = rem;
    }
    return x != 0 ? x : 1;
}

FrameRate RationalFrameRateHelper::fromFloat(double fps, double tolerance) noexcept {
    if (!std::isfinite(fps) || fps <= 0.0) {
        return FrameRate{30, 1};
    }

    // Check standard frame rates within tolerance
    for (const auto& candidate : standardRates()) {
        if (std::abs(fps - candidate.value) <= tolerance) {
            return candidate.rate;
        }
    }

    // Integer fps
    if (std::abs(fps - std::round(fps)) < 1e-6) {
        return FrameRate{static_cast<int32_t>(std::round(fps)), 1};
    }

    // Arbitrary rational approximation with 1,000,000 denominator
    constexpr int64_t ARBITRARY_DENOMINATOR = 1'000'000;
    int64_t scaledNumerator = static_cast<int64_t>(std::round(fps * static_cast<double>(ARBITRARY_DENOMINATOR)));
    int64_t div = gcd(scaledNumerator, ARBITRARY_DENOMINATOR);

    return FrameRate{
        static_cast<int32_t>(scaledNumerator / div),
        static_cast<int32_t>(ARBITRARY_DENOMINATOR / div)
    };
}

int64_t RationalFrameRateHelper::roundFrameTicks(int64_t ticks, const FrameRate& fps) noexcept {
    if (fps.numerator <= 0 || fps.denominator <= 0) return ticks;
    double frameDurationTicks = (static_cast<double>(TICKS_PER_SECOND) * static_cast<double>(fps.denominator))
                                / static_cast<double>(fps.numerator);
    if (frameDurationTicks <= 0.0) return ticks;
    int64_t frameIndex = static_cast<int64_t>(std::round(static_cast<double>(ticks) / frameDurationTicks));
    return static_cast<int64_t>(std::round(static_cast<double>(frameIndex) * frameDurationTicks));
}

int64_t RationalFrameRateHelper::floorFrameTicks(int64_t ticks, const FrameRate& fps) noexcept {
    if (fps.numerator <= 0 || fps.denominator <= 0) return ticks;
    double frameDurationTicks = (static_cast<double>(TICKS_PER_SECOND) * static_cast<double>(fps.denominator))
                                / static_cast<double>(fps.numerator);
    if (frameDurationTicks <= 0.0) return ticks;
    int64_t frameIndex = static_cast<int64_t>(std::floor(static_cast<double>(ticks) / frameDurationTicks));
    return static_cast<int64_t>(std::round(static_cast<double>(frameIndex) * frameDurationTicks));
}

int64_t RationalFrameRateHelper::lastFrameTicks(int64_t durationTicks, const FrameRate& fps) noexcept {
    if (durationTicks <= 0) return 0;
    if (fps.numerator <= 0 || fps.denominator <= 0) return durationTicks;
    double frameDurationTicks = (static_cast<double>(TICKS_PER_SECOND) * static_cast<double>(fps.denominator))
                                / static_cast<double>(fps.numerator);
    int64_t frameTicks = static_cast<int64_t>(std::round(frameDurationTicks));
    if (durationTicks <= frameTicks) return 0;
    return floorFrameTicks(durationTicks - 1, fps);
}

std::optional<double> RationalFrameRateHelper::getHighestImportedVideoFps(
    const std::vector<double>& videoFpsList
) noexcept {
    std::optional<double> highest;
    for (double fps : videoFpsList) {
        if (!std::isfinite(fps) || fps <= 0.0) continue;
        if (!highest.has_value() || fps > highest.value()) {
            highest = fps;
        }
    }
    return highest;
}

std::optional<FrameRate> RationalFrameRateHelper::getRaisedProjectFpsForImportedMedia(
    const FrameRate& currentFps,
    const std::vector<double>& importedVideoFpsList
) noexcept {
    auto highest = getHighestImportedVideoFps(importedVideoFpsList);
    if (!highest.has_value()) return std::nullopt;

    double current = toFloat(currentFps);
    if (highest.value() <= current) {
        return std::nullopt;
    }

    return fromFloat(highest.value());
}

} // namespace catchim::core
