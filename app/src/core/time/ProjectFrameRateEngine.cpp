#include "core/time/ProjectFrameRateEngine.h"
#include <cmath>
#include <algorithm>

namespace catchim::core {

namespace {

struct StandardFpsEntry {
    double value;
    FrameRate rate;
};

const std::vector<StandardFpsEntry>& getStandardFpsTable() noexcept {
    static const std::vector<StandardFpsEntry> s_table = {
        {24000.0 / 1001.0, FrameRate{24000, 1001}}, // 23.976
        {24.0,             FrameRate{24, 1}},
        {25.0,             FrameRate{25, 1}},
        {30000.0 / 1001.0, FrameRate{30000, 1001}}, // 29.97
        {30.0,             FrameRate{30, 1}},
        {48.0,             FrameRate{48, 1}},
        {50.0,             FrameRate{50, 1}},
        {60000.0 / 1001.0, FrameRate{60000, 1001}}, // 59.94
        {60.0,             FrameRate{60, 1}},
        {120.0,            FrameRate{120, 1}}
    };
    return s_table;
}

} // namespace

const std::vector<FpsPreset>& ProjectFrameRateEngine::getFpsPresets() noexcept {
    static const std::vector<FpsPreset> s_presets = {
        {"24",  "24 fps",  24},
        {"25",  "25 fps",  25},
        {"30",  "30 fps",  30},
        {"60",  "60 fps",  60},
        {"120", "120 fps", 120}
    };
    return s_presets;
}

double ProjectFrameRateEngine::frameRateToFloat(const FrameRate& rate) noexcept {
    return rate.toFps();
}

bool ProjectFrameRateEngine::frameRatesEqual(const FrameRate& a, const FrameRate& b) noexcept {
    return a.numerator == b.numerator && a.denominator == b.denominator;
}

int64_t ProjectFrameRateEngine::gcd(int64_t a, int64_t b) noexcept {
    int64_t x = std::abs(a);
    int64_t y = std::abs(b);
    while (y != 0) {
        int64_t rem = x % y;
        x = y;
        y = rem;
    }
    return x != 0 ? x : 1;
}

FrameRate ProjectFrameRateEngine::floatToFrameRate(double fps, double tolerance) noexcept {
    if (!std::isfinite(fps) || fps <= 0.0) {
        return kDefaultFps;
    }

    // Check standard NTSC and integer table
    for (const auto& entry : getStandardFpsTable()) {
        if (std::abs(fps - entry.value) <= tolerance) {
            return entry.rate;
        }
    }

    // Pure integer fps
    if (std::abs(fps - std::round(fps)) < 1e-6) {
        return FrameRate{static_cast<int32_t>(std::round(fps)), 1};
    }

    // Reduce arbitrary fractional fps
    constexpr int64_t kArbitraryDenominator = 1'000'000;
    int64_t scaledNumerator = static_cast<int64_t>(std::round(fps * kArbitraryDenominator));
    int64_t divisor = gcd(scaledNumerator, kArbitraryDenominator);

    return FrameRate{
        static_cast<int32_t>(scaledNumerator / divisor),
        static_cast<int32_t>(kArbitraryDenominator / divisor)
    };
}

std::optional<double> ProjectFrameRateEngine::getHighestImportedVideoFps(
    const std::vector<MediaAssetFpsInfo>& assets
) noexcept {
    std::optional<double> highestFps = std::nullopt;

    for (const auto& asset : assets) {
        if (asset.type != "video" || !asset.fps.has_value()) {
            continue;
        }

        double val = *asset.fps;
        if (!std::isfinite(val) || val <= 0.0) {
            continue;
        }

        if (!highestFps.has_value() || val > *highestFps) {
            highestFps = val;
        }
    }

    return highestFps;
}

std::optional<FrameRate> ProjectFrameRateEngine::getRaisedProjectFpsForImportedMedia(
    const FrameRate& currentFps,
    const std::vector<MediaAssetFpsInfo>& assets
) noexcept {
    auto highestFps = getHighestImportedVideoFps(assets);
    if (!highestFps.has_value()) {
        return std::nullopt;
    }

    double currentFpsFloat = frameRateToFloat(currentFps);
    if (*highestFps <= currentFpsFloat) {
        return std::nullopt;
    }

    return floatToFrameRate(*highestFps);
}

} // namespace catchim::core
