#include "CanvasSizePresets.h"

namespace catchim::render {

const std::vector<editor::CanvasSize>& CanvasSizePresets::defaultCanvasPresets() noexcept {
    static const std::vector<editor::CanvasSize> presets = {
        {1920, 1080},
        {1080, 1920},
        {1080, 1080},
        {1440, 1080},
    };
    return presets;
}

bool CanvasSizePresets::isDefaultPreset(const editor::CanvasSize& size) noexcept {
    for (const auto& preset : defaultCanvasPresets()) {
        if (preset == size) {
            return true;
        }
    }
    return false;
}

double CanvasSizePresets::getAspectRatio(const editor::CanvasSize& size) noexcept {
    if (size.height <= 0) {
        return 0.0;
    }
    return static_cast<double>(size.width) / static_cast<double>(size.height);
}

} // namespace catchim::render
