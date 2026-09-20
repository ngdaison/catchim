#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstdint>

namespace catchim::render {

struct CanvasDimension {
    int32_t width{1920};
    int32_t height{1080};

    bool operator==(const CanvasDimension& other) const = default;
};

struct CanvasPreset {
    std::string name;
    int32_t width{1920};
    int32_t height{1080};
    std::string aspectRatio;
};

class CanvasPresetEngine {
public:
    static const CanvasDimension DEFAULT_CANVAS_SIZE;

    static const std::vector<CanvasPreset>& getStandardPresets();
    static const CanvasPreset& getDefaultPreset();

    static std::pair<int32_t, int32_t> calculateAspectRatioFraction(int32_t width, int32_t height) noexcept;
    static std::string getAspectRatioString(int32_t width, int32_t height);

    static CanvasDimension fitCanvasToMedia(
        int32_t mediaWidth,
        int32_t mediaHeight,
        int32_t maxDimension = 1920
    ) noexcept;

    static int32_t roundToEven(int32_t value) noexcept;
};

} // namespace catchim::render
