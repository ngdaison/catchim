#include "MaskFeatherEngine.h"
#include <cmath>
#include <algorithm>

namespace catchim::render {

Surface2D MaskFeatherEngine::createSurface(int width, int height) {
    Surface2D s;
    if (width > 0 && height > 0) {
        s.width = width;
        s.height = height;
        s.pixels.resize(static_cast<size_t>(width * height * 4), 0);
    }
    return s;
}

double MaskFeatherEngine::clampFeather(double feather, int width, int height) noexcept {
    if (feather <= 0.0) return 0.0;
    const double maxRadius = std::min(width, height) * MAX_FEATHER_FRACTION;
    return std::clamp(feather, 0.0, maxRadius);
}

void MaskFeatherEngine::applyMaskFeather(Surface2D& surface, double featherRadius) {
    if (!surface.isValid() || featherRadius <= 0.0) {
        return;
    }

    const int radius = static_cast<int>(std::round(clampFeather(featherRadius, surface.width, surface.height)));
    if (radius <= 0) {
        return;
    }

    const int w = surface.width;
    const int h = surface.height;
    std::vector<uint8_t> tempAlpha(static_cast<size_t>(w * h), 0);

    // Pass 1: Horizontal box blur on alpha channel
    for (int y = 0; y < h; ++y) {
        int sum = 0;
        int count = 0;

        // Initialize window
        for (int k = -radius; k <= radius; ++k) {
            const int clampedX = std::clamp(k, 0, w - 1);
            sum += surface.pixels[static_cast<size_t>((y * w + clampedX) * 4 + 3)];
            count++;
        }

        for (int x = 0; x < w; ++x) {
            tempAlpha[static_cast<size_t>(y * w + x)] = static_cast<uint8_t>(sum / count);

            // Shift window
            const int removeX = std::clamp(x - radius, 0, w - 1);
            const int addX = std::clamp(x + radius + 1, 0, w - 1);
            sum -= surface.pixels[static_cast<size_t>((y * w + removeX) * 4 + 3)];
            sum += surface.pixels[static_cast<size_t>((y * w + addX) * 4 + 3)];
        }
    }

    // Pass 2: Vertical box blur from tempAlpha back to surface alpha
    for (int x = 0; x < w; ++x) {
        int sum = 0;
        int count = 0;

        for (int k = -radius; k <= radius; ++k) {
            const int clampedY = std::clamp(k, 0, h - 1);
            sum += tempAlpha[static_cast<size_t>(clampedY * w + x)];
            count++;
        }

        for (int y = 0; y < h; ++y) {
            surface.pixels[static_cast<size_t>((y * w + x) * 4 + 3)] = static_cast<uint8_t>(sum / count);

            const int removeY = std::clamp(y - radius, 0, h - 1);
            const int addY = std::clamp(y + radius + 1, 0, h - 1);
            sum -= tempAlpha[static_cast<size_t>(removeY * w + x)];
            sum += tempAlpha[static_cast<size_t>(addY * w + x)];
        }
    }
}

} // namespace catchim::render
