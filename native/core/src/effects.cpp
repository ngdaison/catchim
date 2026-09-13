#include "opencut/effects.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace opencut {

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG_TO_RAD = PI / 180.0f;

float clamp01(float v) noexcept {
    return std::clamp(v, 0.0f, 1.0f);
}

// Convert RGB to HSV
void rgb_to_hsv(float r, float g, float b, float& h, float& s, float& v) noexcept {
    float max_c = std::max({r, g, b});
    float min_c = std::min({r, g, b});
    float delta = max_c - min_c;

    v = max_c;
    if (max_c > 1e-6f) {
        s = delta / max_c;
    } else {
        s = 0.0f;
        h = 0.0f;
        return;
    }

    if (delta < 1e-6f) {
        h = 0.0f;
        return;
    }

    if (r >= max_c) {
        h = (g - b) / delta;
    } else if (g >= max_c) {
        h = 2.0f + (b - r) / delta;
    } else {
        h = 4.0f + (r - g) / delta;
    }

    h *= 60.0f;
    if (h < 0.0f) {
        h += 360.0f;
    }
}

// Convert HSV to RGB
void hsv_to_rgb(float h, float s, float v, float& r, float& g, float& b) noexcept {
    if (s <= 1e-6f) {
        r = g = b = v;
        return;
    }

    h = std::fmod(h, 360.0f);
    if (h < 0.0f) h += 360.0f;
    float h_sector = h / 60.0f;
    int i = static_cast<int>(std::floor(h_sector));
    float f = h_sector - static_cast<float>(i);
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
}

} // namespace

ColorRGBA apply_color_adjustments(ColorRGBA color, const ColorAdjustments& adj) noexcept {
    float r = color.r;
    float g = color.g;
    float b = color.b;

    // 1. Exposure
    if (std::abs(adj.exposure) > 1e-4f) {
        float factor = std::pow(2.0f, adj.exposure);
        r *= factor;
        g *= factor;
        b *= factor;
    }

    // 2. Brightness
    r += adj.brightness;
    g += adj.brightness;
    b += adj.brightness;

    // 3. Contrast
    float contrast_factor = 1.0f + adj.contrast;
    r = (r - 0.5f) * contrast_factor + 0.5f;
    g = (g - 0.5f) * contrast_factor + 0.5f;
    b = (b - 0.5f) * contrast_factor + 0.5f;

    // 4. Temperature & Tint
    r += adj.temperature * 0.1f;
    b -= adj.temperature * 0.1f;
    g += adj.tint * 0.1f;
    r -= adj.tint * 0.05f;
    b -= adj.tint * 0.05f;

    // 5. Saturation
    float luma = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    float sat_factor = 1.0f + adj.saturation;
    r = luma + (r - luma) * sat_factor;
    g = luma + (g - luma) * sat_factor;
    b = luma + (b - luma) * sat_factor;

    // 6. Hue rotation
    if (std::abs(adj.hue_degrees) > 1e-4f) {
        float h = 0.0f, s = 0.0f, v = 0.0f;
        rgb_to_hsv(clamp01(r), clamp01(g), clamp01(b), h, s, v);
        h += adj.hue_degrees;
        hsv_to_rgb(h, s, v, r, g, b);
    }

    // 7. Gamma
    if (std::abs(adj.gamma - 1.0f) > 1e-4f && adj.gamma > 0.0f) {
        float inv_gamma = 1.0f / adj.gamma;
        r = std::pow(clamp01(r), inv_gamma);
        g = std::pow(clamp01(g), inv_gamma);
        b = std::pow(clamp01(b), inv_gamma);
    }

    return {clamp01(r), clamp01(g), clamp01(b), color.a};
}

ColorRGBA invert_color(ColorRGBA color) noexcept {
    return {1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a};
}

float calculate_vignette(Point2D uv, const VignetteParams& params) noexcept {
    float dx = (uv.x - 0.5f);
    float dy = (uv.y - 0.5f) * params.roundness;
    float dist = std::sqrt(dx * dx + dy * dy);

    float inner = (1.0f - params.amount) * 0.5f;
    float outer = inner + std::max(params.softness * 0.5f, 0.01f);

    float t = std::clamp((dist - inner) / (outer - inner), 0.0f, 1.0f);
    return 1.0f - (t * t * (3.0f - 2.0f * t) * params.amount);
}

std::vector<float> generate_gaussian_kernel_1d(int radius, float sigma) {
    if (radius <= 0) return {1.0f};
    int size = 2 * radius + 1;
    std::vector<float> kernel(size);
    float sum = 0.0f;
    float two_sigma_sq = 2.0f * sigma * sigma;

    for (int i = -radius; i <= radius; ++i) {
        float weight = std::exp(-static_cast<float>(i * i) / two_sigma_sq);
        kernel[i + radius] = weight;
        sum += weight;
    }

    if (sum > 0.0f) {
        for (float& w : kernel) {
            w /= sum;
        }
    }

    return kernel;
}

void apply_color_grading_rgba(std::uint32_t* pixels, int width, int height, const ColorAdjustments& adj) noexcept {
    if (!pixels || width <= 0 || height <= 0) return;

    int total = width * height;
    for (int i = 0; i < total; ++i) {
        std::uint32_t p = pixels[i];
        ColorRGBA col{
            (p & 0xFF) / 255.0f,
            ((p >> 8) & 0xFF) / 255.0f,
            ((p >> 16) & 0xFF) / 255.0f,
            ((p >> 24) & 0xFF) / 255.0f
        };

        ColorRGBA res = apply_color_adjustments(col, adj);

        pixels[i] = static_cast<std::uint32_t>(res.r * 255.0f + 0.5f) |
                    (static_cast<std::uint32_t>(res.g * 255.0f + 0.5f) << 8) |
                    (static_cast<std::uint32_t>(res.b * 255.0f + 0.5f) << 16) |
                    (static_cast<std::uint32_t>(res.a * 255.0f + 0.5f) << 24);
    }
}

void apply_gaussian_blur_rgba(std::uint32_t* pixels, int width, int height, int radius, float sigma) {
    if (!pixels || width <= 0 || height <= 0 || radius <= 0) return;

    if (sigma <= 0.0f) {
        sigma = static_cast<float>(radius) / 2.0f;
    }

    auto kernel = generate_gaussian_kernel_1d(radius, sigma);

    std::vector<std::uint32_t> temp(static_cast<std::size_t>(width) * height);

    // Pass 1: Horizontal blur
    for (int y = 0; y < height; ++y) {
        int row_offset = y * width;
        for (int x = 0; x < width; ++x) {
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int sx = std::clamp(x + k, 0, width - 1);
                std::uint32_t p = pixels[row_offset + sx];
                float w = kernel[k + radius];

                r += (p & 0xFF) * w;
                g += ((p >> 8) & 0xFF) * w;
                b += ((p >> 16) & 0xFF) * w;
                a += ((p >> 24) & 0xFF) * w;
            }

            temp[row_offset + x] = static_cast<std::uint32_t>(std::clamp(r, 0.0f, 255.0f) + 0.5f) |
                                  (static_cast<std::uint32_t>(std::clamp(g, 0.0f, 255.0f) + 0.5f) << 8) |
                                  (static_cast<std::uint32_t>(std::clamp(b, 0.0f, 255.0f) + 0.5f) << 16) |
                                  (static_cast<std::uint32_t>(std::clamp(a, 0.0f, 255.0f) + 0.5f) << 24);
        }
    }

    // Pass 2: Vertical blur
    for (int y = 0; y < height; ++y) {
        int row_offset = y * width;
        for (int x = 0; x < width; ++x) {
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int sy = std::clamp(y + k, 0, height - 1);
                std::uint32_t p = temp[sy * width + x];
                float w = kernel[k + radius];

                r += (p & 0xFF) * w;
                g += ((p >> 8) & 0xFF) * w;
                b += ((p >> 16) & 0xFF) * w;
                a += ((p >> 24) & 0xFF) * w;
            }

            pixels[row_offset + x] = static_cast<std::uint32_t>(std::clamp(r, 0.0f, 255.0f) + 0.5f) |
                                    (static_cast<std::uint32_t>(std::clamp(g, 0.0f, 255.0f) + 0.5f) << 8) |
                                    (static_cast<std::uint32_t>(std::clamp(b, 0.0f, 255.0f) + 0.5f) << 16) |
                                    (static_cast<std::uint32_t>(std::clamp(a, 0.0f, 255.0f) + 0.5f) << 24);
        }
    }
}

void apply_vignette_rgba(std::uint32_t* pixels, int width, int height, const VignetteParams& params) noexcept {
    if (!pixels || width <= 0 || height <= 0) return;

    for (int y = 0; y < height; ++y) {
        float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(height);
        int row = y * width;
        for (int x = 0; x < width; ++x) {
            float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(width);
            float vig = calculate_vignette({u, v}, params);
            if (vig >= 0.999f) continue;

            std::uint32_t vig_fixed = static_cast<std::uint32_t>(std::clamp(vig, 0.0f, 1.0f) * 256.0f + 0.5f);
            if (vig_fixed >= 256) continue;

            std::uint32_t p = pixels[row + x];
            std::uint32_t rb = (p & 0x00FF00FF);
            std::uint32_t g  = (p & 0x0000FF00);

            rb = ((rb * vig_fixed) >> 8) & 0x00FF00FF;
            g  = ((g * vig_fixed) >> 8) & 0x0000FF00;

            pixels[row + x] = rb | g | (p & 0xFF000000);
        }
    }
}

void apply_chroma_key_rgba(std::uint32_t* pixels, int width, int height,
                           float key_r, float key_g, float key_b,
                           float similarity, float smoothness, float spill) noexcept {
    if (!pixels || width <= 0 || height <= 0) return;

    int total = width * height;
    float sim = std::clamp(similarity, 0.0f, 1.0f);
    float smooth = std::clamp(smoothness, 0.001f, 1.0f);
    const float inv_sqrt3 = 0.577350269f; // 1.0f / sqrt(3)
    const float inv_smooth = 1.0f / smooth;

    for (int i = 0; i < total; ++i) {
        std::uint32_t p = pixels[i];
        if ((p & 0xFF000000) == 0) continue; // Early exit for completely transparent pixel

        float r = (p & 0xFF) / 255.0f;
        float g = ((p >> 8) & 0xFF) / 255.0f;
        float b = ((p >> 16) & 0xFF) / 255.0f;
        float a = ((p >> 24) & 0xFF) / 255.0f;

        float dr = r - key_r;
        float dg = g - key_g;
        float db = b - key_b;
        float dist = std::sqrt(dr * dr + dg * dg + db * db) * inv_sqrt3;

        float mask = 1.0f;
        if (dist < sim) {
            mask = 0.0f;
        } else if (dist < sim + smooth) {
            float t = (dist - sim) * inv_smooth;
            mask = t * t * (3.0f - 2.0f * t); // smoothstep
        }

        // Spill suppression
        if (spill > 0.0f) {
            float other_avg = (r + b) * 0.5f;
            if (g > other_avg) {
                g = g * (1.0f - spill) + other_avg * spill;
            }
        }

        a *= mask;

        pixels[i] = static_cast<std::uint32_t>(r * 255.0f + 0.5f) |
                    (static_cast<std::uint32_t>(g * 255.0f + 0.5f) << 8) |
                    (static_cast<std::uint32_t>(b * 255.0f + 0.5f) << 16) |
                    (static_cast<std::uint32_t>(a * 255.0f + 0.5f) << 24);
    }
}

} // namespace opencut
