#include "opencut/compositor.hpp"

#include <algorithm>
#include <cmath>

namespace opencut {

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG_TO_RAD = PI / 180.0f;

float clamp01(float v) noexcept {
    return std::clamp(v, 0.0f, 1.0f);
}

float channel_blend(float b, float s, BlendMode mode) noexcept {
    switch (mode) {
        case BlendMode::Normal:
            return s;
        case BlendMode::Darken:
            return std::min(b, s);
        case BlendMode::Multiply:
            return b * s;
        case BlendMode::ColorBurn:
            return (s <= 0.0f) ? 0.0f : clamp01(1.0f - (1.0f - b) / s);
        case BlendMode::Lighten:
            return std::max(b, s);
        case BlendMode::Screen:
            return b + s - b * s;
        case BlendMode::PlusLighter:
            return clamp01(b + s);
        case BlendMode::ColorDodge:
            return (s >= 1.0f) ? 1.0f : clamp01(b / (1.0f - s));
        case BlendMode::Overlay:
            return (b < 0.5f) ? (2.0f * b * s) : (1.0f - 2.0f * (1.0f - b) * (1.0f - s));
        case BlendMode::SoftLight:
            if (s <= 0.5f) {
                return b - (1.0f - 2.0f * s) * b * (1.0f - b);
            } else {
                float d = (b <= 0.25f) ? (((16.0f * b - 12.0f) * b + 4.0f) * b) : std::sqrt(b);
                return b + (2.0f * s - 1.0f) * (d - b);
            }
        case BlendMode::HardLight:
            return (s < 0.5f) ? (2.0f * b * s) : (1.0f - 2.0f * (1.0f - b) * (1.0f - s));
        case BlendMode::Difference:
            return std::abs(b - s);
        case BlendMode::Exclusion:
            return b + s - 2.0f * b * s;
        default:
            return s;
    }
}

} // namespace

Matrix3x3 Matrix3x3::identity() noexcept {
    return Matrix3x3{};
}

Matrix3x3 Matrix3x3::from_transform(const QuadTransform& transform) noexcept {
    float rad = transform.rotation_degrees * DEG_TO_RAD;
    float cos_r = std::cos(rad);
    float sin_r = std::sin(rad);

    float sx = (transform.flip_x ? -1.0f : 1.0f) * transform.width;
    float sy = (transform.flip_y ? -1.0f : 1.0f) * transform.height;

    Matrix3x3 m;
    m.m[0][0] = cos_r * sx;
    m.m[0][1] = -sin_r * sy;
    m.m[0][2] = transform.center_x;

    m.m[1][0] = sin_r * sx;
    m.m[1][1] = cos_r * sy;
    m.m[1][2] = transform.center_y;

    m.m[2][0] = 0.0f;
    m.m[2][1] = 0.0f;
    m.m[2][2] = 1.0f;

    return m;
}

Point2D Matrix3x3::transform_point(Point2D p) const noexcept {
    float x = m[0][0] * p.x + m[0][1] * p.y + m[0][2];
    float y = m[1][0] * p.x + m[1][1] * p.y + m[1][2];
    float w = m[2][0] * p.x + m[2][1] * p.y + m[2][2];
    if (std::abs(w) > 1e-7f) {
        return {x / w, y / w};
    }
    return {x, y};
}

Matrix3x3 Matrix3x3::multiply(const Matrix3x3& other) const noexcept {
    Matrix3x3 result;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            result.m[r][c] = m[r][0] * other.m[0][c] +
                             m[r][1] * other.m[1][c] +
                             m[r][2] * other.m[2][c];
        }
    }
    return result;
}

Matrix3x3 Matrix3x3::inverse() const noexcept {
    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    if (std::abs(det) < 1e-9f) {
        return identity();
    }

    float inv_det = 1.0f / det;
    Matrix3x3 res;

    res.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * inv_det;
    res.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * inv_det;
    res.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * inv_det;

    res.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * inv_det;
    res.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * inv_det;
    res.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * inv_det;

    res.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * inv_det;
    res.m[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * inv_det;
    res.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * inv_det;

    return res;
}

ColorRGBA blend_colors(ColorRGBA base, ColorRGBA layer, BlendMode mode, float opacity) noexcept {
    float eff_opacity = clamp01(opacity * layer.a);

    float blended_r = channel_blend(base.r, layer.r, mode);
    float blended_g = channel_blend(base.g, layer.g, mode);
    float blended_b = channel_blend(base.b, layer.b, mode);

    float final_r = base.r * (1.0f - eff_opacity) + blended_r * eff_opacity;
    float final_g = base.g * (1.0f - eff_opacity) + blended_g * eff_opacity;
    float final_b = base.b * (1.0f - eff_opacity) + blended_b * eff_opacity;
    float final_a = clamp01(base.a + eff_opacity * (1.0f - base.a));

    return {clamp01(final_r), clamp01(final_g), clamp01(final_b), final_a};
}

void clear_buffer_rgba(std::uint32_t* buffer, int width, int height, ColorRGBA clear_color) noexcept {
    if (!buffer || width <= 0 || height <= 0) return;

    std::uint8_t r = static_cast<std::uint8_t>(clamp01(clear_color.r) * 255.0f + 0.5f);
    std::uint8_t g = static_cast<std::uint8_t>(clamp01(clear_color.g) * 255.0f + 0.5f);
    std::uint8_t b = static_cast<std::uint8_t>(clamp01(clear_color.b) * 255.0f + 0.5f);
    std::uint8_t a = static_cast<std::uint8_t>(clamp01(clear_color.a) * 255.0f + 0.5f);
    std::uint32_t packed = static_cast<std::uint32_t>(r) |
                           (static_cast<std::uint32_t>(g) << 8) |
                           (static_cast<std::uint32_t>(b) << 16) |
                           (static_cast<std::uint32_t>(a) << 24);

    std::fill_n(buffer, static_cast<std::size_t>(width) * height, packed);
}

void composite_layer_rgba(
    std::uint32_t* dest, int dest_w, int dest_h,
    const std::uint32_t* src, int src_w, int src_h,
    const QuadTransform& transform,
    float opacity,
    BlendMode mode,
    const float* mask_alpha
) noexcept {
    if (!dest || !src || dest_w <= 0 || dest_h <= 0 || src_w <= 0 || src_h <= 0) return;
    if (opacity <= 0.001f) return;

    Matrix3x3 world = Matrix3x3::from_transform(transform);
    Matrix3x3 inv = world.inverse();

    // Compute destination axis-aligned bounding box of transformed quad
    Point2D c0 = world.transform_point({-0.5f, -0.5f});
    Point2D c1 = world.transform_point({0.5f, -0.5f});
    Point2D c2 = world.transform_point({0.5f, 0.5f});
    Point2D c3 = world.transform_point({-0.5f, 0.5f});

    float min_x_f = std::min({c0.x, c1.x, c2.x, c3.x});
    float max_x_f = std::max({c0.x, c1.x, c2.x, c3.x});
    float min_y_f = std::min({c0.y, c1.y, c2.y, c3.y});
    float max_y_f = std::max({c0.y, c1.y, c2.y, c3.y});

    int min_x = std::clamp(static_cast<int>(std::floor(min_x_f)), 0, dest_w - 1);
    int max_x = std::clamp(static_cast<int>(std::ceil(max_x_f)), 0, dest_w - 1);
    int min_y = std::clamp(static_cast<int>(std::floor(min_y_f)), 0, dest_h - 1);
    int max_y = std::clamp(static_cast<int>(std::ceil(max_y_f)), 0, dest_h - 1);

    if (min_x > max_x || min_y > max_y) return;

    for (int dy = min_y; dy <= max_y; ++dy) {
        int row_offset = dy * dest_w;
        for (int dx = min_x; dx <= max_x; ++dx) {
            Point2D pt = inv.transform_point({static_cast<float>(dx), static_cast<float>(dy)});
            if (pt.x < -0.5f || pt.x >= 0.5f || pt.y < -0.5f || pt.y >= 0.5f) {
                continue;
            }

            float u = (pt.x + 0.5f) * src_w;
            float v = (pt.y + 0.5f) * src_h;

            int sx = std::clamp(static_cast<int>(u), 0, src_w - 1);
            int sy = std::clamp(static_cast<int>(v), 0, src_h - 1);

            std::uint32_t src_p = src[sy * src_w + sx];
            float src_a = ((src_p >> 24) & 0xFF) / 255.0f;
            if (src_a <= 0.001f) continue;

            float eff_op = opacity;
            if (mask_alpha != nullptr) {
                eff_op *= mask_alpha[sy * src_w + sx];
                if (eff_op <= 0.001f) continue;
            }

            std::uint32_t dst_p = dest[row_offset + dx];

            ColorRGBA base{
                (dst_p & 0xFF) / 255.0f,
                ((dst_p >> 8) & 0xFF) / 255.0f,
                ((dst_p >> 16) & 0xFF) / 255.0f,
                ((dst_p >> 24) & 0xFF) / 255.0f
            };

            ColorRGBA layer{
                (src_p & 0xFF) / 255.0f,
                ((src_p >> 8) & 0xFF) / 255.0f,
                ((src_p >> 16) & 0xFF) / 255.0f,
                src_a
            };

            ColorRGBA result = blend_colors(base, layer, mode, eff_op);

            std::uint8_t out_r = static_cast<std::uint8_t>(result.r * 255.0f + 0.5f);
            std::uint8_t out_g = static_cast<std::uint8_t>(result.g * 255.0f + 0.5f);
            std::uint8_t out_b = static_cast<std::uint8_t>(result.b * 255.0f + 0.5f);
            std::uint8_t out_a = static_cast<std::uint8_t>(result.a * 255.0f + 0.5f);

            dest[row_offset + dx] = static_cast<std::uint32_t>(out_r) |
                                   (static_cast<std::uint32_t>(out_g) << 8) |
                                   (static_cast<std::uint32_t>(out_b) << 16) |
                                   (static_cast<std::uint32_t>(out_a) << 24);
        }
    }
}

} // namespace opencut
