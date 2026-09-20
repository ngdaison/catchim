#pragma once

#include "core/time/TimelineTime.h"
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace catchim::core {

struct FpsPreset {
    std::string value;
    std::string label;
    int32_t fps{30};

    bool operator==(const FpsPreset& other) const noexcept = default;
};

struct MediaAssetFpsInfo {
    std::string type; // "video", "audio", "image"
    std::optional<double> fps{std::nullopt};

    bool operator==(const MediaAssetFpsInfo& other) const noexcept = default;
};

class ProjectFrameRateEngine {
public:
    static constexpr FrameRate kDefaultFps{30, 1};
    static constexpr double kStandardTolerance = 0.01;

    // Returns standard presets for UI dropdowns (24 fps, 25 fps, 30 fps, 60 fps, 120 fps)
    static const std::vector<FpsPreset>& getFpsPresets() noexcept;

    // Converts rational FrameRate to double floating-point FPS
    static double frameRateToFloat(const FrameRate& rate) noexcept;

    // Exact equality between two rational FrameRates
    static bool frameRatesEqual(const FrameRate& a, const FrameRate& b) noexcept;

    // Converts float FPS to rational FrameRate matching NTSC standards (23.976, 29.97, 59.94) or GCD-reduced rational
    static FrameRate floatToFrameRate(double fps, double tolerance = kStandardTolerance) noexcept;

    // Greatest Common Divisor
    static int64_t gcd(int64_t a, int64_t b) noexcept;

    // Scans imported assets, filters for video items, and finds the highest valid FPS
    static std::optional<double> getHighestImportedVideoFps(
        const std::vector<MediaAssetFpsInfo>& assets
    ) noexcept;

    // Checks if imported media has higher FPS than current project, returning raised FrameRate if so
    static std::optional<FrameRate> getRaisedProjectFpsForImportedMedia(
        const FrameRate& currentFps,
        const std::vector<MediaAssetFpsInfo>& assets
    ) noexcept;
};

} // namespace catchim::core
