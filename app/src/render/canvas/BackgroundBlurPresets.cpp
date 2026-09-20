#include "BackgroundBlurPresets.h"

#include <algorithm>

namespace catchim::render {

const std::vector<BlurPreset>& BackgroundBlurPresets::getPresets() {
    static const std::vector<BlurPreset> s_presets = {
        {"Light", 100.0},
        {"Medium", 200.0},
        {"Heavy", 500.0}
    };
    return s_presets;
}

std::optional<BlurPreset> BackgroundBlurPresets::findPresetByLabel(const std::string& label) {
    for (const auto& preset : getPresets()) {
        if (preset.label == label) {
            return preset;
        }
    }
    return std::nullopt;
}

bool BackgroundBlurPresets::isValidIntensity(double intensity) noexcept {
    return intensity >= 0.0 && intensity <= 1000.0;
}

double BackgroundBlurPresets::clampIntensity(double intensity) noexcept {
    return std::max(0.0, std::min(1000.0, intensity));
}

} // namespace catchim::render
