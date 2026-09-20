#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

namespace catchim::render {

struct Surface2D {
    int width{0};
    int height{0};
    std::vector<uint8_t> pixels; // RGBA8

    bool isValid() const noexcept {
        return width > 0 && height > 0 && pixels.size() == static_cast<size_t>(width * height * 4);
    }
};

class MaskFeatherEngine {
public:
    static constexpr double MAX_FEATHER_FRACTION = 0.5;

    static Surface2D createSurface(int width, int height);

    static double clampFeather(double feather, int width, int height) noexcept;

    static void applyMaskFeather(Surface2D& surface, double featherRadius);
};

} // namespace catchim::render
