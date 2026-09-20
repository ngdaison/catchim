#pragma once

#include <optional>
#include <string>
#include <vector>

namespace catchim::render {

struct BlurPreset {
    std::string label;
    double value;

    bool operator==(const BlurPreset& other) const noexcept {
        return label == other.label && value == other.value;
    }
};

class BackgroundBlurPresets {
public:
    static constexpr double DEFAULT_BACKGROUND_BLUR_INTENSITY = 10.0;
    static constexpr const char* DEFAULT_BACKGROUND_COLOR = "#000000";

    static const std::vector<BlurPreset>& getPresets();
    static std::optional<BlurPreset> findPresetByLabel(const std::string& label);
    static bool isValidIntensity(double intensity) noexcept;
    static double clampIntensity(double intensity) noexcept;
};

} // namespace catchim::render
