#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace opencut {

enum class BlendMode : std::uint32_t {
    Normal = 0,
    Darken = 1,
    Multiply = 2,
    ColorBurn = 3,
    Lighten = 4,
    Screen = 5,
    PlusLighter = 6,
    ColorDodge = 7,
    Overlay = 8,
    SoftLight = 9,
    HardLight = 10,
    Difference = 11,
    Exclusion = 12,
    Hue = 13,
    Saturation = 14,
    Color = 15,
    Luminosity = 16,
};

struct Point2D {
    float x = 0.0f;
    float y = 0.0f;
};

struct ColorRGBA {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct QuadTransform {
    float center_x = 0.0f;
    float center_y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float rotation_degrees = 0.0f;
    bool flip_x = false;
    bool flip_y = false;
};

struct Matrix3x3 {
    // Row-major 3x3 transformation matrix
    float m[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };

    static Matrix3x3 identity() noexcept;
    static Matrix3x3 from_transform(const QuadTransform& transform) noexcept;

    [[nodiscard]] Point2D transform_point(Point2D p) const noexcept;
    [[nodiscard]] Matrix3x3 multiply(const Matrix3x3& other) const noexcept;
    [[nodiscard]] Matrix3x3 inverse() const noexcept;
};

struct LayerMask {
    std::string texture_id;
    float feather = 0.0f;
    bool inverted = false;
};

struct Layer {
    std::string texture_id;
    QuadTransform transform;
    float opacity = 1.0f;
    BlendMode blend_mode = BlendMode::Normal;
    LayerMask mask;
};

struct Frame {
    std::uint32_t width = 1920;
    std::uint32_t height = 1080;
    ColorRGBA clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<Layer> layers;
};

ColorRGBA blend_colors(ColorRGBA base, ColorRGBA layer, BlendMode mode, float opacity) noexcept;

void clear_buffer_rgba(std::uint32_t* buffer, int width, int height, ColorRGBA clear_color) noexcept;

void composite_layer_rgba(
    std::uint32_t* dest, int dest_w, int dest_h,
    const std::uint32_t* src, int src_w, int src_h,
    const QuadTransform& transform,
    float opacity,
    BlendMode mode,
    const float* mask_alpha = nullptr
) noexcept;

} // namespace opencut
