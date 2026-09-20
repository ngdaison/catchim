#include "render/background/BackgroundPresets.h"

namespace catchim::render {

static const std::vector<BackgroundBlurPreset> kBlurPresets = {
    {"Light", 100},
    {"Medium", 200},
    {"Heavy", 500}
};

const std::vector<BackgroundBlurPreset>& BackgroundPresets::getBlurPresets() noexcept {
    return kBlurPresets;
}

std::optional<BackgroundBlurPreset> BackgroundPresets::findBlurPreset(const std::string& label) noexcept {
    for (const auto& preset : kBlurPresets) {
        if (preset.label == label) {
            return preset;
        }
    }
    return std::nullopt;
}

} // namespace catchim::render
