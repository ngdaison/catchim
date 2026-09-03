#pragma once

#include "opencut/compositor.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

namespace opencut {

struct ColorAdjustments {
    float brightness = 0.0f;    // [-1, 1]
    float contrast = 0.0f;      // [-1, 1]
    float saturation = 0.0f;    // [-1, 1]
    float exposure = 0.0f;      // [-2, 2]
    float temperature = 0.0f;   // [-1, 1]
    float tint = 0.0f;          // [-1, 1]
    float hue_degrees = 0.0f;   // [-180, 180]
    float gamma = 1.0f;         // (0, 3]
};

struct VignetteParams {
    float amount = 0.5f;        // [0, 1]
    float softness = 0.5f;      // (0, 1]
    float roundness = 1.0f;     // aspect-ratio ratio
};

[[nodiscard]] ColorRGBA apply_color_adjustments(ColorRGBA color, const ColorAdjustments& adj) noexcept;

[[nodiscard]] ColorRGBA invert_color(ColorRGBA color) noexcept;

[[nodiscard]] float calculate_vignette(Point2D uv, const VignetteParams& params) noexcept;

[[nodiscard]] std::vector<float> generate_gaussian_kernel_1d(int radius, float sigma);

} // namespace opencut
