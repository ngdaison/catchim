#include "render/canvas/CanvasPresetEngine.h"
#include <numeric>
#include <algorithm>
#include <cmath>

namespace catchim::render {

const CanvasDimension CanvasPresetEngine::DEFAULT_CANVAS_SIZE{1920, 1080};

const std::vector<CanvasPreset>& CanvasPresetEngine::getStandardPresets() {
    static const std::vector<CanvasPreset> presets = {
        {"1080p Landscape (16:9)", 1920, 1080, "16:9"},
        {"1080p Portrait (9:16)",  1080, 1920, "9:16"},
        {"Square (1:1)",           1080, 1080, "1:1"},
        {"Classic TV (4:3)",       1440, 1080, "4:3"},
        {"1440p QHD (16:9)",       2560, 1440, "16:9"},
        {"4K UHD (16:9)",          3840, 2160, "16:9"}
    };
    return presets;
}

const CanvasPreset& CanvasPresetEngine::getDefaultPreset() {
    return getStandardPresets()[0];
}

std::pair<int32_t, int32_t> CanvasPresetEngine::calculateAspectRatioFraction(int32_t width, int32_t height) noexcept {
    if (width <= 0 || height <= 0) {
        return {16, 9};
    }

    int32_t g = std::gcd(width, height);
    if (g == 0) return {16, 9};

    int32_t num = width / g;
    int32_t den = height / g;

    // Handle common near-fractions (e.g., 8:5 for 16:10, 64:27 for 21:9)
    if (num == 8 && den == 5) return {16, 10};
    if (num == 64 && den == 27) return {21, 9};

    return {num, den};
}

std::string CanvasPresetEngine::getAspectRatioString(int32_t width, int32_t height) {
    auto [num, den] = calculateAspectRatioFraction(width, height);
    return std::to_string(num) + ":" + std::to_string(den);
}

int32_t CanvasPresetEngine::roundToEven(int32_t value) noexcept {
    if (value <= 2) return 2;
    return value + (value % 2);
}

CanvasDimension CanvasPresetEngine::fitCanvasToMedia(
    int32_t mediaWidth,
    int32_t mediaHeight,
    int32_t maxDimension
) noexcept {
    if (mediaWidth <= 0 || mediaHeight <= 0) {
        return DEFAULT_CANVAS_SIZE;
    }

    double scale = 1.0;
    if (mediaWidth > maxDimension || mediaHeight > maxDimension) {
        scale = std::min(
            static_cast<double>(maxDimension) / static_cast<double>(mediaWidth),
            static_cast<double>(maxDimension) / static_cast<double>(mediaHeight)
        );
    }

    int32_t w = roundToEven(static_cast<int32_t>(std::round(mediaWidth * scale)));
    int32_t h = roundToEven(static_cast<int32_t>(std::round(mediaHeight * scale)));

    return {w, h};
}

} // namespace catchim::render
