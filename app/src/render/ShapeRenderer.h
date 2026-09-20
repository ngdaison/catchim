#pragma once

#include <cstdint>
#include <algorithm>
#include <cmath>

namespace catchim::render {

struct ColorRGBA {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    static ColorRGBA transparent() { return {0, 0, 0, 0}; }
    static ColorRGBA black() { return {0, 0, 0, 255}; }
    static ColorRGBA white() { return {255, 255, 255, 255}; }
    static ColorRGBA fromRgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return {r, g, b, a};
    }
};

class ShapeRenderer {
public:
    // Background fills
    static void drawSolid(
        uint8_t* buffer,
        int32_t w,
        int32_t h,
        ColorRGBA color
    ) noexcept;

    static void drawLinearGradient(
        uint8_t* buffer,
        int32_t w,
        int32_t h,
        ColorRGBA startColor,
        ColorRGBA endColor,
        double angleDeg = 0.0
    ) noexcept;

    static void drawRadialGradient(
        uint8_t* buffer,
        int32_t w,
        int32_t h,
        ColorRGBA centerColor,
        ColorRGBA edgeColor,
        int32_t radius = -1
    ) noexcept;

    // Vector shapes
    static void drawRectangle(
        uint8_t* buffer,
        int32_t w,
        int32_t h,
        int32_t rx,
        int32_t ry,
        int32_t rw,
        int32_t rh,
        ColorRGBA fill,
        ColorRGBA stroke = ColorRGBA::transparent(),
        int32_t strokeWidth = 0,
        int32_t cornerRadius = 0
    ) noexcept;

    static void drawEllipse(
        uint8_t* buffer,
        int32_t w,
        int32_t h,
        int32_t cx,
        int32_t cy,
        int32_t radiusX,
        int32_t radiusY,
        ColorRGBA fill,
        ColorRGBA stroke = ColorRGBA::transparent(),
        int32_t strokeWidth = 0
    ) noexcept;

private:
    static void blendPixel(
        uint8_t* dst,
        ColorRGBA src
    ) noexcept;
};

} // namespace catchim::render
