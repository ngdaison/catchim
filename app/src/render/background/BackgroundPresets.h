#pragma once

#include <string>
#include <vector>
#include <optional>
#include <algorithm>

namespace catchim::render {

struct BackgroundBlurPreset {
    std::string label;
    int value{0};

    bool operator==(const BackgroundBlurPreset& other) const = default;
};

/**
 * @brief Constants and preset lookups for canvas background blur and color.
 * Corresponds to web/src/background/blur.ts and web/src/background/color.ts.
 */
class BackgroundPresets {
public:
    static constexpr int DEFAULT_BACKGROUND_BLUR_INTENSITY = 10;
    static inline const std::string DEFAULT_BACKGROUND_COLOR = "#000000";

    static const std::vector<BackgroundBlurPreset>& getBlurPresets() noexcept;

    static std::optional<BackgroundBlurPreset> findBlurPreset(const std::string& label) noexcept;

    static int clampBlurIntensity(int intensity) noexcept {
        return std::clamp(intensity, 0, 1000);
    }
};

} // namespace catchim::render
