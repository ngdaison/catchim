#pragma once

#include "core/ids/Ids.h"
#include <string>
#include <vector>
#include <string_view>
#include <cstdint>

namespace catchim::editor {

enum class CanvasBackgroundType {
    Color,
    Blur,
    Gradient,
    Transparent
};

struct BlurIntensityPreset {
    const char* label;
    int32_t value;
};

struct CanvasSizePreset {
    const char* label;
    int32_t width;
    int32_t height;
    const char* aspectRatio;
};

enum class FitMode {
    Contain,
    Cover
};

struct FittedRect {
    double x{0.0};
    double y{0.0};
    double width{0.0};
    double height{0.0};
    double scale{1.0};

    bool operator==(const FittedRect& other) const = default;
};

struct CanvasBackgroundConfig {
    CanvasBackgroundType type{CanvasBackgroundType::Color};
    std::string color{"#000000"};
    int32_t blurIntensity{10};
    std::string gradientStart{"#000000"};
    std::string gradientEnd{"#333333"};
    double gradientAngleDegrees{0.0};
};

class CanvasBackgroundEngine {
public:
    static constexpr const char* DEFAULT_BACKGROUND_COLOR = "#000000";
    static constexpr int32_t DEFAULT_BACKGROUND_BLUR_INTENSITY = 10;
    static constexpr int32_t DEFAULT_CANVAS_WIDTH = 1920;
    static constexpr int32_t DEFAULT_CANVAS_HEIGHT = 1080;

    static const std::vector<BlurIntensityPreset>& blurPresets() noexcept;
    static const std::vector<CanvasSizePreset>& canvasPresets() noexcept;

    /**
     * @brief Detects standard aspect ratio strings ("16:9", "9:16", "1:1", "4:3", "21:9")
     * or computes the simplified ratio string.
     */
    static std::string detectAspectRatio(int32_t width, int32_t height) noexcept;

    /**
     * @brief Fits a media rectangle (width, height) into a canvas of (canvasWidth, canvasHeight)
     * using Contain or Cover scaling. Centered at canvas center.
     */
    static FittedRect fitRectIntoCanvas(
        double mediaWidth,
        double mediaHeight,
        double canvasWidth,
        double canvasHeight,
        FitMode mode = FitMode::Contain
    ) noexcept;

    /**
     * @brief Calculates uniform fit scale factor to fit a canvas into a viewport area.
     */
    static double calculateFitScale(
        double canvasWidth,
        double canvasHeight,
        double viewportWidth,
        double viewportHeight,
        double padding = 20.0
    ) noexcept;
};

} // namespace catchim::editor
