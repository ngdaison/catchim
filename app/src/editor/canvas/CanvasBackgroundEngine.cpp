#include "CanvasBackgroundEngine.h"
#include <numeric>
#include <cmath>
#include <algorithm>

namespace catchim::editor {

const std::vector<BlurIntensityPreset>& CanvasBackgroundEngine::blurPresets() noexcept {
    static const std::vector<BlurIntensityPreset> presets = {
        {"Light", 100},
        {"Medium", 200},
        {"Heavy", 500}
    };
    return presets;
}

const std::vector<CanvasSizePreset>& CanvasBackgroundEngine::canvasPresets() noexcept {
    static const std::vector<CanvasSizePreset> presets = {
        {"1080p Landscape", 1920, 1080, "16:9"},
        {"1080p Portrait (Shorts/Reels)", 1080, 1920, "9:16"},
        {"Square", 1080, 1080, "1:1"},
        {"Classic TV", 1440, 1080, "4:3"},
        {"2K QHD", 2560, 1440, "16:9"},
        {"4K UHD", 3840, 2160, "16:9"},
        {"Ultrawide", 2560, 1080, "21:9"}
    };
    return presets;
}

std::string CanvasBackgroundEngine::detectAspectRatio(int32_t width, int32_t height) noexcept {
    if (width <= 0 || height <= 0) {
        return "16:9";
    }

    double ratio = static_cast<double>(width) / static_cast<double>(height);

    if (std::abs(ratio - (16.0 / 9.0)) < 0.02) return "16:9";
    if (std::abs(ratio - (9.0 / 16.0)) < 0.02) return "9:16";
    if (std::abs(ratio - 1.0) < 0.02) return "1:1";
    if (std::abs(ratio - (4.0 / 3.0)) < 0.02) return "4:3";
    if (std::abs(ratio - (21.0 / 9.0)) < 0.05) return "21:9";

    int32_t g = std::gcd(width, height);
    if (g > 1) {
        return std::to_string(width / g) + ":" + std::to_string(height / g);
    }

    return std::to_string(width) + ":" + std::to_string(height);
}

FittedRect CanvasBackgroundEngine::fitRectIntoCanvas(
    double mediaWidth,
    double mediaHeight,
    double canvasWidth,
    double canvasHeight,
    FitMode mode
) noexcept {
    if (mediaWidth <= 0.0 || mediaHeight <= 0.0 || canvasWidth <= 0.0 || canvasHeight <= 0.0) {
        return FittedRect{0.0, 0.0, canvasWidth, canvasHeight, 1.0};
    }

    double scaleX = canvasWidth / mediaWidth;
    double scaleY = canvasHeight / mediaHeight;
    double scale = (mode == FitMode::Contain) ? std::min(scaleX, scaleY) : std::max(scaleX, scaleY);

    double w = mediaWidth * scale;
    double h = mediaHeight * scale;
    double x = (canvasWidth - w) / 2.0;
    double y = (canvasHeight - h) / 2.0;

    return FittedRect{x, y, w, h, scale};
}

double CanvasBackgroundEngine::calculateFitScale(
    double canvasWidth,
    double canvasHeight,
    double viewportWidth,
    double viewportHeight,
    double padding
) noexcept {
    double availW = std::max(10.0, viewportWidth - (padding * 2.0));
    double availH = std::max(10.0, viewportHeight - (padding * 2.0));

    if (canvasWidth <= 0.0 || canvasHeight <= 0.0) {
        return 1.0;
    }

    double scaleX = availW / canvasWidth;
    double scaleY = availH / canvasHeight;
    return std::min(scaleX, scaleY);
}

} // namespace catchim::editor
